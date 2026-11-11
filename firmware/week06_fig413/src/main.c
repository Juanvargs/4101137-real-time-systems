#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/sys/printk.h>

/* Fig. 4.13: tau1=(C=2 ms,T=D=5 ms), tau2=(C=4 ms,T=D=7 ms). */
#define TAU1_PERIOD_MS  5
#define TAU1_EXEC_US    2000
#define TAU2_PERIOD_MS  7
#define TAU2_EXEC_US    4000
#define FIRST_RELEASE_MS 100
#define RUN_SECONDS      60
#define TASK_STACK_SIZE  1536

static const struct gpio_dt_spec tau1_pin =
	GPIO_DT_SPEC_GET(DT_NODELABEL(tau1_instr), gpios);
static const struct gpio_dt_spec tau2_pin =
	GPIO_DT_SPEC_GET(DT_NODELABEL(tau2_instr), gpios);

struct periodic_task {
	const struct gpio_dt_spec *pin;
	uint32_t period_ms;
	uint32_t execution_us;
	atomic_t deadline_misses;
};

static struct periodic_task tau1 = {
	.pin = &tau1_pin,
	.period_ms = TAU1_PERIOD_MS,
	.execution_us = TAU1_EXEC_US,
};

static struct periodic_task tau2 = {
	.pin = &tau2_pin,
	.period_ms = TAU2_PERIOD_MS,
	.execution_us = TAU2_EXEC_US,
};

static int64_t epoch_ms;
static uint32_t epoch_cycles;

/* Calibrated CPU-time computation: k_busy_wait() counts wall-clock time,
 * which keeps running while the thread is preempted. */
static uint32_t iters_per_100us;

static __noinline void spin(uint32_t iters)
{
	for (volatile uint32_t i = 0; i < iters; i++) {
	}
}

static void compute_us(uint32_t us)
{
	spin((uint32_t)(((uint64_t)iters_per_100us * us) / 100U));
}

static void calibrate(void)
{
	const uint32_t n = 200000;
	uint64_t best_ns = UINT64_MAX;

	for (int trial = 0; trial < 5; trial++) {
		uint32_t c0 = k_cycle_get_32();

		spin(n);
		uint64_t ns = k_cyc_to_ns_floor64(k_cycle_get_32() - c0);

		if (ns < best_ns) {
			best_ns = ns;
		}
	}
	iters_per_100us = (uint32_t)(((uint64_t)n * 100000U) / best_ns);
}

#ifdef CONFIG_SCHED_DEADLINE
/* EDF compares deadlines only among threads with equal static priority. */
#define TAU1_PRIORITY 3
#define TAU2_PRIORITY 3
#else
/* RM: the shorter-period task gets the higher static priority. */
#define TAU1_PRIORITY 2
#define TAU2_PRIORITY 3
#endif

static void periodic_task(void *p1, void *p2, void *p3)
{
	struct periodic_task *task = p1;
	int64_t release_ms = epoch_ms;
	uint32_t release_cycles = epoch_cycles;
	uint32_t period_cycles = k_ms_to_cyc_ceil32(task->period_ms);

	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	while (true) {
		/* Absolute release times avoid adding each job's execution to T. */
		k_sleep(K_TIMEOUT_ABS_MS(release_ms));

		gpio_pin_set_dt(task->pin, 1);
	        compute_us(task->execution_us);
		gpio_pin_set_dt(task->pin, 0);

		uint32_t job_deadline = release_cycles + period_cycles;
		int32_t lateness = (int32_t)(k_cycle_get_32() - job_deadline);

		if (lateness > 0) {
			atomic_inc(&task->deadline_misses);
		}

		/* Prepare the next job's absolute deadline before blocking. */
		release_ms += task->period_ms;
		release_cycles += period_cycles;
#ifdef CONFIG_SCHED_DEADLINE
		uint32_t next_deadline = release_cycles + period_cycles;
		k_thread_absolute_deadline_set(k_current_get(), (int)next_deadline);
#endif
	}
}

/* Start suspended; main assigns the first EDF deadlines before making them ready. */
K_THREAD_DEFINE(tau1_tid, TASK_STACK_SIZE, periodic_task, &tau1, NULL, NULL,
		TAU1_PRIORITY, 0, SYS_FOREVER_MS);
K_THREAD_DEFINE(tau2_tid, TASK_STACK_SIZE, periodic_task, &tau2, NULL, NULL,
		TAU2_PRIORITY, 0, SYS_FOREVER_MS);

int main(void)
{
	if (!gpio_is_ready_dt(&tau1_pin) || !gpio_is_ready_dt(&tau2_pin)) {
		printk("ERROR: instrumentation GPIO controller not ready\n");
		return 0;
	}

	gpio_pin_configure_dt(&tau1_pin, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&tau2_pin, GPIO_OUTPUT_INACTIVE);
	calibrate();
	printk("calibration: %u iterations per 100 us\n", iters_per_100us);

#ifdef CONFIG_SCHED_DEADLINE
	printk("Fig. 4.13 mode: EDF; tau1/tau2 static priority=%d\n",
	       TAU1_PRIORITY);
#else
	printk("Fig. 4.13 mode: RM; tau1 priority=%d, tau2 priority=%d\n",
	       TAU1_PRIORITY, TAU2_PRIORITY);
#endif
	printk("tau1 C=2ms T=D=5ms GPIO4; tau2 C=4ms T=D=7ms GPIO5\n");
	printk("Starting first jobs in %d ms; miss counters reported after %d s\n",
	       FIRST_RELEASE_MS, RUN_SECONDS);

	/* Give both jobs a common release epoch and set initial absolute deadlines. */
	epoch_ms = k_uptime_get() + FIRST_RELEASE_MS;
	epoch_cycles = k_cycle_get_32() + k_ms_to_cyc_ceil32(FIRST_RELEASE_MS);
#ifdef CONFIG_SCHED_DEADLINE
	k_thread_absolute_deadline_set(tau1_tid,
		(int)(epoch_cycles + k_ms_to_cyc_ceil32(TAU1_PERIOD_MS)));
	k_thread_absolute_deadline_set(tau2_tid,
		(int)(epoch_cycles + k_ms_to_cyc_ceil32(TAU2_PERIOD_MS)));
#endif
	k_thread_start(tau1_tid);
	k_thread_start(tau2_tid);

	/* Main sleeps during the experiment, then reports misses outside the capture. */
	k_sleep(K_MSEC(FIRST_RELEASE_MS + RUN_SECONDS * 1000));
	printk("RESULT after %d s: tau1_misses=%ld tau2_misses=%ld\n",
	       RUN_SECONDS, atomic_get(&tau1.deadline_misses),
	       atomic_get(&tau2.deadline_misses));

	while (true) {
		k_sleep(K_FOREVER);
	}
	return 0;
}
