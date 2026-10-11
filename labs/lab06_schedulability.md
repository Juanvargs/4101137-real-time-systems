# Week 6 — Schedulability: the theory, put to the test
> - **Reading:** [READINGS.md](../READINGS.md), week 6 (includes the optional Lee & Seshia)
> - **Module:** 3
> - **Problem Set 1 goes out today** (exercises from chs. 2 and 4; due at the workshop, week 8).

**From:** Eng. Samuel Cifuentes — *"Daniela is asking whether she can add two more
sensors to the node. Today you answer like engineers: not 'let's try and see', but
'utilization lands at X, the test says Y'. And since I don't trust theorems any
more than I trust you, reproduce the famous case for me: a task set that RM misses
and EDF meets — on our board, not in the book. And before anyone says yes to
Daniela: two requirements fail whatever U says. `calib` spins for 400 ms against a
1 s deadline, and the e-stop waits for control. Fix both at the source."*

| Stakeholder | Their question | How this session answers it |
|---|---|---|
| **Samuel** | Does more load fit on the node? | U computed from measured `C_i` + the applicable test |
| **Daniela** | What if irrigation is *a little* late? | The loop with induced jitter: the degradation, measured |
| **Edward** | Can one command still eat the CPU a deadline needs? | `calib` sleeps instead of spinning: CPU measured before and after |

## What you'll measure

| Measurement | Your value | Prediction |
|---|---|---|
| U of the synthetic task set (3 tasks) | ____ | designed at U ≈ 0.97 |
| Deadline miss under RM (which task, when?) | ____ | theory says: yes, it misses (Fig. 4.13) |
| Deadline miss under EDF | ____ | theory says: U ≤ 1 ⇒ it meets |
| Flow-loop error with induced jitter of 0 / 2 / 5 ms | ____ / ____ / ____ | grows with jitter |
| `calib` CPU / duration, before the fix | ____ ms / ____ ms | it spins: CPU = duration |
| `calib` CPU / duration, after the fix | ____ ms / ____ ms | a few ms of CPU; duration barely moves |
| E-stop, detection → valve low, max of 5 trips, before / after | ____ / ____ ms | sum the path in Task D |

## Tasks

### Task A — Fig. 4.13, live
- Implement the task set from the book's Fig. 4.13 (periods and computation times
  given; "computation" is a calibrated busy-wait). Run with RM priorities; capture
  the miss.
- Switch to EDF (`CONFIG_SCHED_DEADLINE=y`, same static priority,
  `k_thread_deadline_set` each period). Verify it meets.
- **Evidence:** two analyzer captures — the miss under RM, the success under EDF.

### Task B — Does the new load fit?
- With the real node's measured `C_i`: compute U, apply Liu & Layland and the
  hyperbolic test. Add "Daniela's two tasks" (parameters given in class) and repeat.
- **Evidence:** the calculation in RET §4, with a verdict and which test backs it.

### Task C — Jitter vs. control (the bridge to control theory)
- Inject artificial jitter into the flow loop (random 0–N ms delay before
  actuating). For N = 0, 2, 5 ms: measure the loop error (deviation from target
  flow, or from the course rig's setpoint).
- **Evidence:** error-vs-jitter table + one sentence: at which N does the loop stop being useful?

### Task D — Fix it at the source (ADR-005)

Task B's verdict covers the periodic set. Two requirements fail whatever U says:

- **REQ-CONS-01.** `calib` needs ≈ 400 ms of CPU within its 1 s deadline, at the
  lowest priority. With Daniela's tasks, how much of that second do the tasks
  above the console leave it?
- **REQ-ESTOP-01.** Sampling detects the overpressure, but the valve only closes
  once control's next run sets the duty to 0 and the PWM step after it writes the
  pin. Add up the worst case from the periods: does it fit in 5 ms?

Measure the *before* case first. Unplug the flow input so `instr_flow` stays
idle, add `instr_set(&instr_flow, estop);` right after the e-stop check in
`task_sampling` to mark the detection, and clip a free analyzer channel on the
valve (GPIO21). `set 3300` trips the e-stop within ~2 s; `set 1500` then `clear`
resets it.

Then fix both where the time goes:

1. In `cmd_calib`, `k_busy_wait(400)` becomes `k_usleep(400)`: the console waits
   instead of spinning. Sleeps round up to the kernel tick, 100 µs on the S3
   (`CONFIG_SYS_CLOCK_TICKS_PER_SEC`). Do 1000 settles still fit in 1 s? What
   would a 1 kHz tick do?
2. In `task_sampling`, the branch that sets `estop` also sets `duty_pct = 0`, so
   the PWM step a few lines below drives the valve low in the same job.

- `instr_cons` now stays high while the console sleeps, so its width is the
  duration, not the CPU: take the CPU from a week-5 trace.
- Redo the fit question and the e-stop sum for the new design.
- **Evidence:** the before/after rows, an e-stop capture of each, and both sums.

## What about FreeRTOS?

FreeRTOS only ships fixed priorities — there is no EDF in the kernel. Task A can't
be reproduced as-is: with U = 0.97 and FreeRTOS, the professional answer is to
redesign the task set until it passes the RM test (or lower U). That constraint is
real in industry — and it's why the book spends half a chapter on RM.

## Deliverables (RET)

- **§2 ADR-005 — E-stop and `calib` fixed at the source:** context (the fit
  question and the e-stop sum), decision, before/after numbers.
- **§4 Schedulability:** U, both tests, verdict on Daniela's load; the `calib`
  fit and the e-stop path, before and after ADR-005.
- **§3 Week-6 evidence:** Fig. 4.13 captures + jitter-vs-control table + Task D's
  before/after captures.

## Rubric (100 pts)

| | pts |
|---|---|
| **Execution** — 4.13 task set missing under RM (10) · EDF meeting (10) · jitter injected (10) · ADR-005 fix (10) | 40 |
| **Evidence** — captures of both regimes (10) · error-vs-jitter table (10) · Task D before/after (10) | 30 |
| **Analysis** — U and tests correctly applied to Daniela's case (15) · control-jitter reading (5) · ADR-005 with the fit and e-stop sums (10) | 30 |
