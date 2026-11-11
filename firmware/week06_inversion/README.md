# Week 6 Task B — Priority inversion

Implementation timing: once low takes the lock and raises GPIO4, one-shot
timers release control after 2 ms and medium after 3 ms while low is executing
CPU work. This keeps the high request and medium preemption inside the low
critical section.

This isolated Zephyr app makes the baseline, semaphore inversion, and priority
inheritance cases repeatable without changing the SoilSense controller.

## Build and flash

From the Zephyr workspace:

```bash
west build -p always -b esp32s3_devkitc/esp32s3/procpu \
  ~/4101137-real-time-systems/firmware/week06_inversion \
  -d build/week06-inversion-s3
west flash -d build/week06-inversion-s3 \
  --esp-device /dev/serial/by-id/usb-1a86_USB_Single_Serial_5C82127093-if00
```

Open the UART console at 115200 baud. The app accepts one-character commands:

- `b`: control response with the lock free.
- `s`: low owns a binary semaphore, control blocks, and medium preempts low.
- `m`: repeat with a `k_mutex`; priority inheritance lets low finish before medium.

## Analyzer wiring

Use a common ground and connect DIO0→GPIO4 (low), DIO1→GPIO5 (control), and
DIO2→GPIO6 (medium). Record the signal after issuing each command. Measure
positive pulse width on DIO1 for control response; DIO0 is the low critical
section, whose width in the mutex case is B; DIO2 marks medium work.

Each low critical section performs 30 ms of calibrated CPU work, medium performs
25 ms, and control performs 1 ms. Since this is CPU work, preemption extends the
wall-clock pulse in the semaphore case. Expected ordering:

The calibration stores `iterations_per_us` as an integer, so the computed loop
rate is truncated. In the recorded build this made the nominal 30 ms, 25 ms,
and 1 ms workloads about 28.15 ms, 23.44 ms, and 0.938 ms respectively. The
analyzer widths in the evidence are the measured values and should be treated as
empirical for this build, not as exact requested CPU times.

- `b`: only the short control response pulse.
- `s`: control requests the held lock; medium runs while low is blocked from CPU;
  control's response is longest.
- `m`: low inherits control's priority, finishes its critical section, then
  control proceeds; medium runs afterward.

Record the actual analyzer measurements; these durations are targets, not
substitutes for measured evidence.
