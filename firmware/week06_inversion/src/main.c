#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#define LOW_PRIORITY       7
#define MEDIUM_PRIORITY    4
#define CONTROL_PRIORITY   2
#define TASK_STACK_SIZE    2048
#define LOW_WORK_US        30000U
#define MEDIUM_WORK_US     25000U
#define CONTROL_WORK_US    1000U

static const struct gpio_dt_spec low_pin =
	GPIO_DT_SPEC_GET(DT_NODELABEL(low_instr), gpios);
static const struct gpio_dt_spec control_pin =
	GPIO_DT_SPEC_GET(DT_NODELABEL(control_instr), gpios);
static const struct gpio_dt_spec medium_pin =
	GPIO_DT_SPEC_GET(DT_NODELABEL(medium_instr), gpios);
static const struct device *const console_uart =
	DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

static void release_control(struct k_timer *timer);
static void release_medium(struct k_timer *timer);

K_SEM_DEFINE(start_low, 0, 1);
K_SEM_DEFINE(start_control, 0, 1);
K_SEM_DEFINE(start_medium, 0, 1);
K_SEM_DEFINE(low_done, 0, 1);
K_SEM_DEFINE(control_done, 0, 1);
K_SEM_DEFINE(medium_done, 0, 1);

K_SEM_DEFINE(binary_lock, 1, 1);
K_MUTEX_DEFINE(pi_lock);
K_TIMER_DEFINE(control_release_timer, release_control, NULL);
K_TIMER_DEFINE(medium_release_timer, release_medium, NULL);

static bool use_mutex;
static uint32_t iterations_per_us;

static void release_control(struct k_timer *timer)
{
	ARG_UNUSED(timer);
	k_sem_give(&start_control);
}

static void release_medium(struct k_timer *timer)
{
	ARG_UNUSED(timer);
	k_sem_give(&start_medium);
}

static void spin(uint32_t iterations)
{
	for (volatile uint32_t i = 0; i < iterations; i++) {
	}
}

/* Calibrate a CPU loop so preemption extends its wall-clock duration. */
static void calibrate_work(void)
{
	const uint32_t iterations = 200000U;
	uint64_t best_ns = UINT64_MAX;

	for (int trial = 0; trial < 5; trial++) {
		uint32_t start = k_cycle_get_32();

		spin(iterations);
		uint64_t elapsed_ns =
			k_cyc_to_ns_floor64(k_cycle_get_32() - start);

		if (elapsed_ns < best_ns) {
			best_ns = elapsed_ns;
		}
	}

	iterations_per_us = (uint32_t)(((uint64_t)iterations * 1000U) /
					 best_ns);
}

static void work_us(uint32_t duration_us)
{
	uint64_t iterations = (uint64_t)iterations_per_us * duration_us;

	spin((uint32_t)iterations);
}

static void lock_take(void)
{
	if (use_mutex) {
		k_mutex_lock(&pi_lock, K_FOREVER);
	} else {
		k_sem_take(&binary_lock, K_FOREVER);
	}
}

static void lock_give(void)
{
	if (use_mutex) {
		k_mutex_unlock(&pi_lock);
	} else {
		k_sem_give(&binary_lock);
	}
}

static void low_thread(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	while (true) {
		k_sem_take(&start_low, K_FOREVER);
		lock_take();
		gpio_pin_set_dt(&low_pin, 1);
		/* Release high and medium while low is doing CPU work in the
		 * critical section. High blocks on the lock; medium then preempts low
		 * with a semaphore, while a mutex boosts low ahead of medium.
		 */
		k_timer_start(&control_release_timer, K_MSEC(2), K_NO_WAIT);
		k_timer_start(&medium_release_timer, K_MSEC(3), K_NO_WAIT);
		work_us(LOW_WORK_US);
		gpio_pin_set_dt(&low_pin, 0);
		lock_give();
		k_sem_give(&low_done);
	}
}

static void control_thread(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	while (true) {
		k_sem_take(&start_control, K_FOREVER);
		gpio_pin_set_dt(&control_pin, 1);
		lock_take();
		work_us(CONTROL_WORK_US);
		lock_give();
		gpio_pin_set_dt(&control_pin, 0);
		k_sem_give(&control_done);
	}
}

static void medium_thread(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	while (true) {
		k_sem_take(&start_medium, K_FOREVER);
		gpio_pin_set_dt(&medium_pin, 1);
		work_us(MEDIUM_WORK_US);
		gpio_pin_set_dt(&medium_pin, 0);
		k_sem_give(&medium_done);
	}
}

K_THREAD_DEFINE(low_tid, TASK_STACK_SIZE, low_thread, NULL, NULL, NULL,
		LOW_PRIORITY, 0, 0);
K_THREAD_DEFINE(medium_tid, TASK_STACK_SIZE, medium_thread, NULL, NULL, NULL,
		MEDIUM_PRIORITY, 0, 0);
K_THREAD_DEFINE(control_tid, TASK_STACK_SIZE, control_thread, NULL, NULL, NULL,
		CONTROL_PRIORITY, 0, 0);

static void run_baseline(void)
{
	use_mutex = false;
	k_sem_give(&start_control);
	k_sem_take(&control_done, K_FOREVER);
	printk("BASELINE complete: semaphore free; GPIO5 marks control response (~1 ms).\n");
}

static void run_contention(bool mutex)
{
	use_mutex = mutex;
	k_sem_give(&start_low);
	k_sem_take(&control_done, K_FOREVER);
	k_sem_take(&low_done, K_FOREVER);
	k_sem_take(&medium_done, K_FOREVER);

	printk("%s complete: GPIO5 is control response; GPIO4 is low critical section B; GPIO6 is medium work.\n",
	       mutex ? "MUTEX-PI" : "SEMAPHORE-INVERSION");
}

int main(void)
{
	if (!gpio_is_ready_dt(&low_pin) || !gpio_is_ready_dt(&control_pin) ||
	    !gpio_is_ready_dt(&medium_pin) ||
	    !device_is_ready(console_uart)) {
		printk("ERROR: GPIO or console UART not ready\n");
		return 0;
	}

	gpio_pin_configure_dt(&low_pin, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&control_pin, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&medium_pin, GPIO_OUTPUT_INACTIVE);
	calibrate_work();
	printk("Week 6 Task B ready; calibrated %u loop iterations/us.\n",
	       iterations_per_us);
	printk("GPIO4=low, GPIO5=control, GPIO6=medium. Type b=baseline, s=semaphore inversion, m=mutex PI.\n");

	/* Keep the command-polling main thread below all three experiment threads. */
	k_thread_priority_set(k_current_get(), 10);
	while (true) {
		unsigned char ch;

		if (uart_poll_in(console_uart, &ch) == 0) {
			if (ch == 'b' || ch == 'B') {
				run_baseline();
			} else if (ch == 's' || ch == 'S') {
				run_contention(false);
			} else if (ch == 'm' || ch == 'M') {
				run_contention(true);
			} else if (ch != '\r' && ch != '\n') {
				printk("Type b, s, or m.\n");
			}
		}
		k_msleep(5);
	}
	return 0;
}
