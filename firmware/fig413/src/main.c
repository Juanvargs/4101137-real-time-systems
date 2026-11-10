/*
 * Week 6 Task A — Buttazzo, Hard Real-Time Computing Systems, Fig. 4.13, live.
 *
 *   tau1: C = 2 ms, T = D = 5 ms
 *   tau2: C = 4 ms, T = D = 7 ms        U = 2/5 + 4/7 = 0.971
 *
 * Default build (prj.conf): fixed priorities assigned rate monotonic, so tau1
 * (shorter period) has the higher priority. Expected: tau2 misses at t = 7 ms.
 *
 * EDF build (-DEXTRA_CONF_FILE=edf.conf): both threads share one static
 * priority and the kernel orders them by the absolute deadline that each job
 * sets with k_thread_absolute_deadline_set(). The deadline of the *next* job is
 * set BEFORE the thread blocks until its next release; setting it after the
 * wake-up is too late, because the scheduler has already chosen with the old
 * deadline and the run looks like RM.
 *
 * "Computation" is a calibrated busy loop that counts loop iterations, so it
 * consumes C of CPU time. k_busy_wait() would not do: it measures wall-clock
 * time, which keeps running while the thread is preempted.
 *
 * Instrumentation (see the board overlay in boards/):
 *   instr_t1 / instr_t2: high from the start of a job's computation to its end.
 *     While tau1's pin is high inside a tau2 pulse, tau2 is preempted.
 *   instr_miss: short pulse when a job finishes after its absolute deadline.
 *
 * The logic analyzer is the judge of the numbers. The console report uses
 * kernel ticks (100 us on the S3), so its response times are coarse.
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/sys/printk.h>

#ifdef CONFIG_SCHED_DEADLINE
#define PRIO_T1 1
#define PRIO_T2 1 /* EDF: same static priority, deadline breaks the tie */
#define POLICY  "EDF"
#else
#define PRIO_T1 1 /* RM: shorter period -> higher priority (lower number) */
#define PRIO_T2 2
#define POLICY  "RM"
#endif
#define PRIO_REPORT 10 /* below both tasks under either policy */

#define RUN_SECONDS 60
#define STACK_SIZE  1024

#define INSTR(node) GPIO_DT_SPEC_GET_OR(DT_NODELABEL(node), gpios, {0})
static const struct gpio_dt_spec pin_t1 = INSTR(instr_t1);
static const struct gpio_dt_spec pin_t2 = INSTR(instr_t2);
static const struct gpio_dt_spec pin_miss = INSTR(instr_miss);

static inline void pin_set(const struct gpio_dt_spec *p, int v)
{
	if (p->port) {
		gpio_pin_set_dt(p, v);
	}
}

/* ---- Calibrated computation ---------------------------------------------- */

static uint32_t iters_per_100us;

static __noinline void spin(uint32_t iters)
{
	for (volatile uint32_t i = 0; i < iters; i++) {
	}
}

static void spin_us(uint32_t us)
{
	spin((uint32_t)(((uint64_t)iters_per_100us * us) / 100U));
}

/* Time a fixed number of iterations several times and keep the fastest run:
 * the fastest one is the one least disturbed by interrupts. */
static void calibrate(void)
{
	const uint32_t n = 200000;
	uint64_t best_ns = UINT64_MAX;

	for (int trial = 0; trial < 5; trial++) {
		uint32_t c0 = k_cycle_get_32();

		spin(n);
		uint32_t dc = k_cycle_get_32() - c0;
		uint64_t ns = k_cyc_to_ns_floor64(dc);

		if (ns < best_ns) {
			best_ns = ns;
		}
	}
	iters_per_100us = (uint32_t)(((uint64_t)n * 100000U) / best_ns);
}

/* ---- Tasks ---------------------------------------------------------------- */

struct task {
	const char *name;
	uint32_t c_us;
	uint32_t t_ms;
	const struct gpio_dt_spec *pin;
	atomic_t jobs;
	atomic_t misses;
	atomic_t max_resp_us;
	atomic_t first_miss_ms; /* release-relative time of the first miss, -1 = none */
};

static struct task tasks[] = {
	{ .name = "tau1", .c_us = 2000, .t_ms = 5, .pin = &pin_t1,
	  .first_miss_ms = ATOMIC_INIT(-1) },
	{ .name = "tau2", .c_us = 4000, .t_ms = 7, .pin = &pin_t2,
	  .first_miss_ms = ATOMIC_INIT(-1) },
};

static int64_t t0_ticks; /* common synchronous release of both tasks */

#ifdef CONFIG_SCHED_DEADLINE
/* Absolute deadline given in kernel ticks, converted to the hardware cycle
 * counter that k_thread_absolute_deadline_set() compares against. */
static void set_abs_deadline(int64_t abs_ticks)
{
	int64_t d = abs_ticks - k_uptime_ticks();
	int32_t dc = (d >= 0) ? (int32_t)k_ticks_to_cyc_floor64(d)
			      : -(int32_t)k_ticks_to_cyc_floor64(-d);

	k_thread_absolute_deadline_set(k_current_get(),
				       (int)(k_cycle_get_32() + (uint32_t)dc));
}
#endif

static void task_thread(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);
	struct task *t = p1;
	const int64_t period = k_ms_to_ticks_ceil64(t->t_ms);
	int64_t release = t0_ticks;

#ifdef CONFIG_SCHED_DEADLINE
	set_abs_deadline(release + period); /* first job, before blocking */
#endif
	k_sleep(K_TIMEOUT_ABS_TICKS(release));

	for (;;) {
		pin_set(t->pin, 1);
		spin_us(t->c_us);
		pin_set(t->pin, 0);

		int64_t now = k_uptime_ticks();
		int64_t deadline = release + period;
		int64_t resp_us = k_ticks_to_us_floor64(now - release);

		atomic_inc(&t->jobs);
		if (resp_us > atomic_get(&t->max_resp_us)) {
			atomic_set(&t->max_resp_us, (atomic_val_t)resp_us);
		}
		if (now > deadline) {
			atomic_inc(&t->misses);
			atomic_cas(&t->first_miss_ms, -1,
				   (atomic_val_t)k_ticks_to_ms_floor64(deadline - t0_ticks));
			pin_set(&pin_miss, 1);
			k_busy_wait(5);
			pin_set(&pin_miss, 0);
		}

		/* Releases stay on the grid t0 + k*T: a late job does not shift
		 * the next release, it only shortens the sleep (possibly to 0). */
		release += period;
#ifdef CONFIG_SCHED_DEADLINE
		set_abs_deadline(release + period); /* next job, before blocking */
#endif
		k_sleep(K_TIMEOUT_ABS_TICKS(release));
	}
}

K_THREAD_STACK_DEFINE(stack_t1, STACK_SIZE);
K_THREAD_STACK_DEFINE(stack_t2, STACK_SIZE);
static struct k_thread thread_t1, thread_t2;

/* ---- Report (lowest priority: it only gets the CPU the tasks leave) ------- */

static void report(void)
{
	int s = (int)(k_ticks_to_ms_floor64(k_uptime_ticks() - t0_ticks) / 1000);

	printk("[%s t=%2ds]", POLICY, s);
	for (int i = 0; i < ARRAY_SIZE(tasks); i++) {
		struct task *t = &tasks[i];

		printk("  %s jobs=%ld misses=%ld maxR=%ld us first_miss=%ld ms",
		       t->name, (long)atomic_get(&t->jobs),
		       (long)atomic_get(&t->misses),
		       (long)atomic_get(&t->max_resp_us),
		       (long)atomic_get(&t->first_miss_ms));
	}
	printk("\n");
}

static void report_thread(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	for (int s = 1; s <= RUN_SECONDS; s++) {
		k_sleep(K_TIMEOUT_ABS_TICKS(t0_ticks + k_ms_to_ticks_ceil64(1000 * s)));
		report();
	}
	printk("[%s] %d s run complete; tasks keep running\n", POLICY, RUN_SECONDS);
}

K_THREAD_STACK_DEFINE(stack_report, STACK_SIZE);
static struct k_thread thread_report;

int main(void)
{
	const struct gpio_dt_spec *pins[] = { &pin_t1, &pin_t2, &pin_miss };

	for (int i = 0; i < ARRAY_SIZE(pins); i++) {
		if (pins[i]->port && gpio_is_ready_dt(pins[i])) {
			gpio_pin_configure_dt(pins[i], GPIO_OUTPUT_INACTIVE);
		}
	}

	calibrate();
	printk("\nFig. 4.13 under %s: tau1 C=2 T=5 ms, tau2 C=4 T=7 ms, U=0.971\n",
	       POLICY);
	printk("calibration: %u iterations per 100 us\n", iters_per_100us);

	/* Common release 100 ms from now, so both threads are created and
	 * blocked before the first job: a synchronous start, as in the book. */
	t0_ticks = k_uptime_ticks() + k_ms_to_ticks_ceil64(100);

	k_thread_create(&thread_t1, stack_t1, STACK_SIZE, task_thread,
			&tasks[0], NULL, NULL, PRIO_T1, 0, K_NO_WAIT);
	k_thread_name_set(&thread_t1, "tau1");
	k_thread_create(&thread_t2, stack_t2, STACK_SIZE, task_thread,
			&tasks[1], NULL, NULL, PRIO_T2, 0, K_NO_WAIT);
	k_thread_name_set(&thread_t2, "tau2");
	k_thread_create(&thread_report, stack_report, STACK_SIZE, report_thread,
			NULL, NULL, NULL, PRIO_REPORT, 0, K_NO_WAIT);
	k_thread_name_set(&thread_report, "report");

	return 0;
}
