# RET — Timing Evidence Report

## Document control

| Field | Value |
|---|---|
| Document ID | RET-STR-001 |
| Project | SoilSense Control / SoilSense Hub |
| Course | Real-Time Systems — UNAL 4101137 |
| Team | Individual — Juan Pablo Vargas Cordoba |
| Authors | Juan Pablo Vargas Cordoba |
| Status | Draft — Week 5 tracing and instrumentation-cost evidence recorded |
| Version | 0.7 |
| Date | 2026-11-10 |
| Repository baseline | Week 5 tracing evidence committed (`87d4067`, 2026-11-04) |
| Starter board | NUCLEO-G474RE — student-owned; course baseline differs |
| Board/debug identifier | STLINK-V3 `003400303232510139353236` |
| Week 3–4 comparison MCU | ESP32-C6 DevKitC compatible, 4 MB flash |
| Week 5 tracing MCU | ESP32-S3 DevKitC — identifier not yet recorded (OI-006) |

### Revision history

| Version | Date | Author | Change |
|---|---|---|---|
| 0.1 | 2026-09-20 | Juan Pablo Vargas Cordoba | Professional RET structure and first environment evidence |
| 0.2 | 2026-09-21 | Juan Pablo Vargas Cordoba | Week 1 hardware evidence and scenario-derived initial task set |
| 0.3 | 2026-09-26 | Juan Pablo Vargas Cordoba | Week 2 superloop, flash-accelerator, and blocking-command evidence |
| 0.4 | 2026-10-10 | Juan Pablo Vargas Cordoba | Added observed widths for control, console, telemetry, and flow batch |
| 0.5 | 2026-10-15 | Juan Pablo Vargas Cordoba | Added ESP32-C6 superloop versus sampling-thread timing evidence |
| 0.6 | 2026-10-27 | Juan Pablo Vargas Cordoba | Added complete kernel A/B, visible hand-off cost, stack high-water marks, and ADR-001 |
| 0.7 | 2026-11-10 | Juan Pablo Vargas Cordoba | Added Week 5 tracing evidence, annotated traces, trace-vs-GPIO analysis and jitter records; fixed requirement links, evidence numbering, stale §4.1 inputs, and open items |

## Purpose and evidence rule

This living report demonstrates that the system satisfies its timing requirements.
The required argument is:

`requirement -> task -> analysis -> measured evidence -> verdict`

Every timing claim shall cite an evidence ID. A result without its platform,
firmware revision, configuration, load, instrument, and observation interval is not
accepted as timing evidence.

## 1. The system and its task set

### 1.1 Scope

SoilSense Control is a Zephyr-based irrigation controller. It contains periodic
sensing and control activities, an overpressure safety response, telemetry, a
command console, and an optional display. SoilSense Hub, introduced in week 10 and
developed as the final project, adds PREEMPT_RT Linux, a local GUI, routing, and a
hard pump-station loop.

Sources:

- `PROJECT_SCENARIO.md` — product context and reference requirements.
- `firmware/superloop/` — initial implementation to be measured.
- `READINGS.md` — analysis methods and course scope.

### 1.2 Timing requirements

The requirements below are initial engineering requirements. Values marked `TBD`
must be fixed before the corresponding verification; they must not be inferred from
successful runs.

| ID | Requirement (EARS style) | Type | Status | Planned verification |
|---|---|---|---|---|
| REQ-CTRL-01 | While the system is irrigating, the control loop shall be released every 10 ms and shall complete no later than its next release. | Hard | Kernel mitigation observed; strict deadline not demonstrated | GPIO + logic analyzer; RTA |
| REQ-CTRL-02 | When measured pressure exceeds the safety threshold, the system shall command the valve/pump to its safe state within 5 ms. | Hard | Draft | Instrumented input-to-output latency |
| REQ-CTRL-03 | While control is enabled, the system shall sample the pressure input every 1 ms and shall complete each sampling job before its next release. | Hard | Kernel mitigation observed; strict deadline still missed by 5.6 µs under `calib` | GPIO + logic analyzer |
| REQ-CTRL-04 | While the control node is operational, the system shall publish telemetry of its current state at least once every 1000 ms. | Soft | Observed; detailed deadline analysis pending | GPIO + logic analyzer |
| REQ-CTRL-05 | When a valid console command is received, the system shall produce a usable response within `TBD`; a response after that limit may be discarded. | Firm | Incomplete | UART timestamps + trace |
| REQ-CTRL-06 | While a firm console command is executing, the system shall continue to satisfy all hard sampling deadlines. | Hard | Superloop FAIL; kernel reduces interference to 6.2 µs observed jitter, but strict hard deadline remains unproven | GPIO + logic analyzer + serial status |

### 1.3 Task set

`C_i` is the execution time of task `i`. It remains `____` until it is measured
under a declared protocol. A future maximum observed value will be reported as
`C_obs,max`; it shall not be called WCET without a justified upper-bound method.

| Task | Requirement | Type | Activation/period | Deadline | Measured `C_i` | Jitter bound | Priority/policy | Measurement method |
|---|---|---|---:|---:|---:|---:|---|---|
| Sensor sampling | REQ-CTRL-03 | Hard | Periodic, 1 ms (1 kHz) | 1 ms | NUCLEO: `C_obs,max` 3 µs baseline; 4 µs cache-off; 12 µs during `calib`. C6: `C_obs,max` 2.7 µs in Week 3. | C6 full kernel: 6 µs idle, 6.2 µs with `calib` observed max | `sampling_tid`, preemptive prio. 2 | GPIO DIO0 + AD2 logic analyzer (all `C_i` values) |
| Control loop | REQ-CTRL-01 | Hard | Periodic, 10 ms | 10 ms | `C_obs,max` 0.7 µs (300 NUCLEO runs, 10 MHz capture) | C6 full kernel period with `calib`: 10.003 ms observed max | `control_tid`, preemptive prio. 3 | GPIO DIO1 + AD2 logic analyzer |
| Overpressure emergency stop | REQ-CTRL-02 | Hard | Event-triggered | < 5 ms | Shared with sampling; standalone response not measured | N/A | TBD | Instrumented input-to-output latency |
| Command console | REQ-CTRL-05, REQ-CTRL-06 | Firm | Event-triggered/sporadic | TBD | `status` path: 4 us to 6.6010 ms across 8 received bytes; `calib` blocks the loop for approximately 414 ms; neither is a WCET | N/A | `console_tid`, preemptive prio. 9 | GPIO DIO2 + AD2 logic analyzer; UART for command input |
| Telemetry | REQ-CTRL-04 | Soft | Periodic, 1 s | 1 s (derived from task period) | `C_obs,max` 5.7770 ms (30 runs) | TBD | `telemetry_tid`, preemptive prio. 8 | GPIO DIO3 + AD2 logic analyzer |
| Flow batch | N/A | N/A | Every 100 flow pulses | TBD | `C_obs,max` 44 us (30 batches at 100 Hz stimulus) | N/A | private `flow_wq`, preemptive prio. 5 | GPIO DIO4 + AD2 logic analyzer |
| Display HMI | N/A | Soft | Periodic, 500 ms when enabled | TBD | Not enabled on this run | N/A | `display_tid`, preemptive prio. 7 | GPIO DIO5 + AD2 logic analyzer (planned) |

Instrument rule (from Week 5): every `C_i` above was measured by GPIO pulse width
on the AD2 logic analyzer, which does not perturb the target. CTF trace
timestamps include the tracer's own cost (EV-W05-JITTER-ON-004/OFF-005) and are
used only to explain the order of events, never as a `C_i` source.

No unmeasured execution time, telemetry period, console deadline, sensor deadline,
or jitter bound has been invented; each unresolved value remains explicit until the
course provides it or the team measures and justifies it.

### 1.4 Initial traceability matrix

| Requirement | Implementing task | Analysis | Evidence | Verdict |
|---|---|---|---|---|
| REQ-CTRL-01 | Control loop | Utilization + RTA | EV-W02-TASKS-006 records observed width; EV-W04-AB-001 observes 10.003 ms maximum period during `calib` | FAIL strict; analytical proof pending |
| REQ-CTRL-02 | Overpressure emergency stop | Input-to-output response bound | Pending measurement | PENDING |
| REQ-CTRL-03 | Sensor sampling | Period/jitter analysis | EV-W02-BASELINE-002, EV-W02-CACHE-004, EV-W02-BLOCK-005, EV-W03-BASELINE-002, EV-W03-THREAD-004, EV-W04-AB-001 | FAIL strict; strong kernel mitigation observed |
| REQ-CTRL-04 | Telemetry | Period characterization | EV-W02-TASKS-006 observed 30 publications in 30 s; period statistic pending | INCONCLUSIVE |
| REQ-CTRL-05 | Command console | Response-time characterization | EV-W02-TASKS-006 (`status`) and EV-W02-BLOCK-005; deadline is TBD | INCONCLUSIVE |
| REQ-CTRL-06 | Console versus hard sampling | Non-interference trace | EV-W02-BLOCK-005, EV-W03-CALIB-003, EV-W03-THREAD-004, EV-W04-AB-001 | FAIL strict; sampling interference strongly reduced |

## 2. Architecture Decision Records

No architecture decision is accepted before its alternatives and timing evidence
exist.

### ADR-001 — Control-node execution architecture

- **Status:** Accepted experimentally in Week 4; formal schedulability proof
  remains pending.
- **Context:** The same ESP32-C6 DevKitC-compatible board ran the same
  SoilSense task set before and after the migration. In the Week-3 superloop,
  the synchronous console command `calib` delayed sampling to 403.47 ms and
  control to 412.47 ms. That violates the nominal 1 ms and 10 ms deadlines by
  orders of magnitude.
- **Alternatives:** retain the superloop; use the full multithreaded Zephyr
  design; use a hybrid design that moves only selected tasks.
- **Decision:** Use the full multithreaded Zephyr design: `k_msgq` for sampled
  release timestamps, `k_sem` from sampling to control, a private priority-5
  flow workqueue, and sleeping display/telemetry/console threads. `main`
  performs bring-up and returns.
- **Quantitative justification:** EV-W04-AB-001 measured maximum sampling
  jitter with `calib` of 6.2 µs, versus 402.47 ms in the C6 superloop. Maximum
  control period with `calib` became 10.003 ms, versus 412.47 ms. The measured
  ISR-to-sampling start peak was 10 µs and the visible sampling-to-control
  hand-off maximum was 11.7 µs (EV-W04-COST-002), estimated as 1.17 ms/s at
  100 control releases per second.
- **Consequences and costs:** The design prevents the firm console workload
  from producing hundreds of milliseconds of interference. It introduces IPC,
  priorities, context-switch overhead and per-thread stack budgets. The stack
  measurement found `idle` at 228/256 B (89 %, only 28 B free), so that stack
  needs review before product deployment (EV-W04-STACK-003). The observed
  maxima still exceed strict nominal periods by 5.6 µs for sampling and 3 µs
  for control; therefore this ADR chooses the substantially safer architecture
  but does not claim a formal hard-real-time PASS.

### ADR-002 — Mutual-exclusion policy

- **Status:** Not started; decision due in week 7.
- **Decision:** TBD after measuring blocking and priority inversion.

### ADR-003 — Core allocation for the hard loop

- **Status:** Not started; decision due in week 9.
- **Decision:** TBD after comparing one-core load with AMP isolation.

### ADR-004 — Hub real-time configuration

- **Status:** Not started; decision due in week 12.
- **Decision:** TBD after PREEMPT_RT, CPU affinity, IRQ, and interference tests.

## 3. Evidence

### 3.1 Evidence register

| Evidence ID | Week | Objective | Requirements | Platform | Result | Verdict | Record |
|---|---:|---|---|---|---|---|---|
| EV-W01-ENV-001 | Pre-course | Verify the mandatory no-hardware Zephyr build/run path | N/A | `native_sim/native` | Student build/run successful; stale CMake cache diagnosed | PASS | `evidence/week01/EV-W01-ENV-001.md` |
| EV-W01-BUILD-002 | Week 1 | Build Zephyr `blinky` for the starter board | N/A | NUCLEO-G474RE | Firmware generated; 18,896 B flash and 4,544 B RAM | PASS — build only | `evidence/week01/EV-W01-BUILD-002.md` |
| EV-W01-HW-003 | Week 1 | Flash and execute Zephyr `blinky` on the physical board | N/A | NUCLEO-G474RE | STLINK-V3 programming succeeded; LD2 toggled about once per second | PASS | `evidence/week01/EV-W01-HW-003.md` |
| EV-W01-SERIAL-004 | Week 1 | Verify the modified-message serial iteration cycle | N/A | NUCLEO-G474RE | Modified message observed at 115200 8N1 | PASS | `evidence/week01/EV-W01-SERIAL-004.md` |
| EV-W02-FLOW-001 | Week 2 | Map the superloop, ISRs, and instrumentation GPIOs | REQ-CTRL-01, 03, 06 | NUCLEO-G474RE | Ten-box flow diagram and board pin map | PASS | `evidence/week02/EV-W02-FLOW-001.md` |
| EV-W02-BASELINE-002 | Week 2 | Measure cache-on sampling baseline | REQ-CTRL-03 | NUCLEO-G474RE | Mean 1.0001 ms; max 6.845 ms; `J_max` 5.845 ms | FAIL | `evidence/week02/EV-W02-BASELINE-002.md` |
| EV-W02-FLOWLAT-003 | Week 2 | Characterize flow ISR-to-loop hand-off | N/A | NUCLEO-G474RE | One observed latency approximately 6 us | INCONCLUSIVE | `evidence/week02/EV-W02-FLOWLAT-003.md` |
| EV-W02-CACHE-004 | Week 2 | Compare flash accelerator enabled/disabled | REQ-CTRL-03 | NUCLEO-G474RE | `C_obs,mean` +36.2%; `J_max` 5.845 to 5.857 ms | FAIL | `evidence/week02/EV-W02-CACHE-004.md` |
| EV-W02-BLOCK-005 | Week 2 | Measure blocking `calib` command interference | REQ-CTRL-03, 06 | NUCLEO-G474RE | `backlog_peak` 6 to 414; max period 414.06 ms | FAIL | `evidence/week02/EV-W02-BLOCK-005.md` |
| EV-W02-TASKS-006 | Week 2 | Measure the remaining instrumented task widths | REQ-CTRL-01, 04, 05 | NUCLEO-G474RE | Control max 0.7 us; `status` max 6.6010 ms; telemetry max 5.7770 ms; flow max 44 us | CHARACTERIZED | `evidence/week02/EV-W02-TASKS-006.md` |
| EV-W03-PORT-001 | Week 3 | Port the superloop via Devicetree to ESP32-C6 SuperMini | N/A | ESP32-C6 SuperMini | Overlay-only port, build/flash/serial verified | PASS | `evidence/week03/EV-W03-PORT-001.md` |
| EV-W03-BASELINE-002 | Week 3 | C6 superloop idle sampling baseline | REQ-CTRL-03 | ESP32-C6 DevKitC compatible | Max period 1.1050 ms; observed max jitter 105 us | FAIL strict | `evidence/week03/EV-W03-BASELINE-002.md` |
| EV-W03-CALIB-003 | Week 3 | C6 superloop interference from `calib` | REQ-CTRL-01, 03, 06 | ESP32-C6 DevKitC compatible | Sampling max 403.47 ms; control max 412.47 ms; backlog 403 | FAIL | `evidence/week03/EV-W03-CALIB-003.md` |
| EV-W03-THREAD-004 | Week 3 | C6 priority-thread mitigation | REQ-CTRL-01, 03, 06 | ESP32-C6 DevKitC compatible | Sampling max 1.0020 ms during `calib`; control max 412.31 ms; backlog 0; release-to-start peak 11 us | FAIL strict; mitigation demonstrated | `evidence/week03/EV-W03-THREAD-004.md` |
| EV-W03-CI-008 | Week 3 | Measure C6 sampling execution width | REQ-CTRL-03 | ESP32-C6 DevKitC compatible | `C_obs,max` 2.7 us across 3,000 observed pulses | CHARACTERIZED | `evidence/week03/EV-W03-CI-008.md` |
| EV-W04-AB-001 | Week 4 | Compare C6 superloop and full kernel migration | REQ-CTRL-01, 03, 06 | ESP32-C6 DevKitC compatible | `calib`: sampling jitter 402.47 ms -> 6.2 µs; control max period 412.47 -> 10.003 ms | FAIL strict; strong mitigation demonstrated | `evidence/week04/EV-W04-AB-001.md` |
| EV-W04-COST-002 | Week 4 | Measure visible sampling-to-control hand-off gap | N/A | ESP32-C6 DevKitC compatible | Max observed gap 11.7 µs; 1.17 ms/s estimated at 100 hand-offs/s | CHARACTERIZED | `evidence/week04/EV-W04-COST-002.md` |
| EV-W04-STACK-003 | Week 4 | Measure thread stack high-water marks under load | N/A | ESP32-C6 DevKitC compatible | `idle` 228/256 B (89 %); `control_tid` 236/1024 B | CHARACTERIZED; idle margin needs review | `evidence/week04/EV-W04-STACK-003.md` |
| EV-W05-CPU-001 | Week 5 | Normal-operation per-thread CPU share, including idle and tracer | N/A | ESP32-S3 DevKitC | `idle` 97.99 %; `tracing_thread` 0.79 %; shares normalized from analyzer cycle counts | CHARACTERIZED; cumulative since boot | `evidence/week05/EV-W05-CPU-001.md` |
| EV-W05-PREEMPT-002 | Week 5 | CTF event sequence during `calib` | N/A | ESP32-S3 DevKitC | Console → sampling observed switch interval 4.837 µs; ISR-to-sampling start 20.012 µs; `lat_peak_us` 30 µs | CHARACTERIZED; CTF overhead applies | `evidence/week05/EV-W05-PREEMPT-002.md`; annotated: `EV-W05-PREEMPT-002-annotated.png` |
| EV-W05-CONTROL-003 | Week 5 | Tenth-sample control semaphore release | N/A | ESP32-S3 DevKitC | `semaphore_give` 12.171 ms; `control_tid` switched in 12.195 ms | CHARACTERIZED | `evidence/week05/EV-W05-CONTROL-003-annotated.png`; context in `EV-W05-PREEMPT-002.md` |
| EV-W05-JITTER-ON-004 | Week 5 | Sampling period with tracing enabled | REQ-CTRL-03 | ESP32-S3 DevKitC | Mean 0.99999 ms; min 0.98730 ms; max 1.0065 ms; max absolute deviation 12.7 µs | CHARACTERIZED | `evidence/week05/EV-W05-JITTER-ON-004.md` |
| EV-W05-JITTER-OFF-005 | Week 5 | Sampling period with tracing disabled | REQ-CTRL-03 | ESP32-S3 DevKitC | Mean 1.00000 ms; min 0.99770 ms; max 1.0021 ms; max absolute deviation 2.3 µs | CHARACTERIZED | `evidence/week05/EV-W05-JITTER-OFF-005.md` |

### 3.2 Week 1 — environment and reproducibility

The no-hardware prerequisite is satisfied by EV-W01-ENV-001. The following items
remain open before week 1 is complete:

| Check | Status | Evidence |
|---|---|---|
| `west --version` responds | PASS | EV-W01-ENV-001 |
| `hello_world` builds on `native_sim` | PASS | EV-W01-ENV-001 |
| `hello_world` runs on `native_sim` | PASS | EV-W01-ENV-001 |
| ARM toolchain for the starter board is installed | PASS | EV-W01-ENV-001 |
| Xtensa toolchain for ESP32-S3 is installed | PASS | EV-W01-ENV-001 |
| User belongs to the serial-access group (`dialout`) | PASS | EV-W01-ENV-001 |
| VS Code or the selected editor is available | PASS | GNU nano 7.2 declared as the course editor (OI-005) |
| Starter-board target confirmed | PASS | NUCLEO-G474RE; Zephyr target `nucleo_g474re` |
| `blinky` builds for the physical board | PASS | EV-W01-BUILD-002 |
| `blinky` is flashed and LD2 blinks | PASS | EV-W01-HW-003 |
| Serial console shows a modified message | PASS | EV-W01-SERIAL-004 |
| Team and board identifiers recorded | PASS | RET document control |

### 3.3 Week 2 — provided superloop

#### Measurement conditions

- Board: NUCLEO-G474RE; instrumentation overlay maps D3–D8 to task GPIOs.
- Analyzer: Analog Discovery 2 with WaveForms 3.25.1 Logic; record mode,
  30,000,000 samples at 1 MHz for 30 s (1 us sample resolution).
- Normal baseline workload: no console command and Pattern generator stopped.
- Cache-off run: same source, pins, clock, analyzer configuration, and workload;
  only ART instruction/data caches and Flash prefetch were disabled.
- Blocking run: cache-on firmware, identical capture settings, one `calib`
  command issued through ST-LINK VCP at 115200 8N1.
- Per-task-width runs: cache-on firmware. Control used 30 M samples at 10 MHz
  for 3 s (0.1 us resolution); console, telemetry, and flow used 30 M samples
  at 1 MHz for 30 s (1 us resolution).

#### Timing results

| Measurement | Cache ON baseline | Cache OFF | During `calib` |
|---|---:|---:|---:|
| Sampling mean period | 1.0001 ms | 1.0001 ms | 1.0001 ms |
| Sampling minimum period | 6 us | 7 us | 6 us |
| Sampling maximum period | 6.845 ms | 6.857 ms | 414.06 ms |
| Sampling maximum absolute jitter | 5.845 ms | 5.857 ms | 413.06 ms |
| Sampling mean pulse width | 2.4886 us | 3.3888 us | 2.5071 us |
| Sampling maximum pulse width | 3 us | 4 us | 12 us |
| `backlog_peak` | 6 | Not separately queried | 414 |

The cache-on superloop fails REQ-CTRL-03: its 6.845 ms maximum period is
5.845 ms beyond the 1 ms deadline even before the explicit blocking command.
The `calib` command makes the failure approximately 71 times larger in maximum
jitter (413.06 ms versus 5.845 ms) and raises `backlog_peak` from 6 to 414.
The analyzer and firmware agree because 414 pending 1 ms ticks correspond to
about 414 ms. Cache-off increased mean observed sampling execution time by 36.2%,
but changed the dominant jitter only 12 us; the synchronous loop blocking is the
main cause.

#### Remaining instrumented task widths

| Task / workload | Capture | Count | Mean | Minimum | Maximum observed |
|---|---|---:|---:|---:|---:|
| Control | 3 s normal run, DIO1 at 10 MHz | 300 | 0.63300 us | 0.60000 us | 0.70000 us |
| Console | one `status` command, DIO2 at 1 MHz | 8 bytes | 0.85037 ms | 4 us | 6.6010 ms |
| Telemetry | 30 s idle run, DIO3 at 1 MHz | 30 | 5.7682 ms | 5.6900 ms | 5.7770 ms |
| Flow batch | 30 s, 100 Hz DIO6-to-D9 stimulus, DIO4 at 1 MHz | 30 | 34.033 us | 33 us | 44 us |

The console statistics intentionally mix short character-receive jobs with the
larger end-of-line `status` handler, so their mean is not a generic console
execution time. The 6.6010 ms maximum is the largest observed normal-`status`
path in this protocol. The 300 control executions in 3 s agree with its nominal
10 ms activation interval. These are `C_obs` observations, not WCET bounds.

#### Flow hand-off characterization

AD2 DIO6 drove Nucleo D9/PC7 at 100 Hz and DIO4 observed flow-batch work. For
one selected 100th-pulse event, service began about 6 us after the DIO6 rising
edge. This is a characterized observed latency, not a guarantee, because no
flow-service deadline is allocated yet.

#### Evidence and limitations

See `evidence/week02/README.md` and the five evidence records cited above.
The preserved artifacts are WaveForms screenshots; no transition-data export or
workspace was retained. Results therefore support the shown statistics and
verdicts, but do not constitute a formal WCET proof.

### 3.4 Week 3 — ESP32-C6 port and first sampling thread

#### Platform adaptation

The course handout names an ESP32-S3 and an STM32L476RG. The physical boards
used in this repository are an ESP32-C6 DevKitC-compatible board (4 MB flash)
and the previously measured NUCLEO-G474RE. Therefore this is a transparent
comparison of those actual boards, not a claim that C6 measurements are S3
measurements. The C6 port uses the board-specific Devicetree overlay
`boards/esp32c6_devkitc_hpcore.overlay`; no application-C change was required
for the port itself. The extra flash-node declaration corrects the Zephyr
board's 8 MB default to the 4 MB module physically present.

#### Three-column comparison

`J_obs,max` is the largest observed absolute deviation from the nominal
sampling period. The C6 threaded idle interval is five consecutive 6 s captures
at 5 MHz (30 s total), because the AD2's 30 M-sample memory covers 6 s at that
resolution.

| Measurement | NUCLEO-G474RE superloop (Week 2) | ESP32-C6 superloop | ESP32-C6 + sampling thread |
|---|---:|---:|---:|
| Max sampling jitter, >= 30 s idle | 5.845 ms | 105 us | 2 us |
| Max sampling jitter, `calib` active | 413.06 ms | 402.47 ms | 2 us |
| `backlog_peak`, `calib` active | 414 ticks | 403 ticks | 0 ticks |
| `lat_peak_us`, tick -> sampling-thread start | — | — | 11 us |
| Max control period, `calib` active | Not measured in Week 2 | 412.47 ms | 412.31 ms |

The values in the two C6 `calib` columns are from simultaneous DIO0/DIO1
captures at 5 MHz. The initial 1 MHz DIO1 capture is deliberately excluded
from the control-period result because it could not resolve its short pulses.

#### Reading of the result

The periodic ISR does not perform an ADC conversion. It timestamps each release
and puts it in `tick_q`. `sampling_thread` is priority 2, whereas `main` is
priority 10; therefore, when `calib` holds `main`, a tick makes the higher-
priority sampling thread run first. This changes sampling's worst observed
period during `calib` from 403.47 ms to 1.0020 ms and keeps the queue empty.

Control still has an approximately 412 ms maximum period because it remains in
`main`. `calib` is also executed synchronously by the console path in `main`,
so no thread can run `task_control()` until calibration returns. The change is
thus a targeted sampling mitigation, not a complete threaded redesign.

The C6 runs at a different clock and uses a different memory/serial subsystem
than the G474. Clock rate alone is not a timing argument: the maximum period
also includes preemption, blocking I/O, flash/cache behavior, and driver work.
For that reason the RET records measured maxima rather than attributing a
telemetry difference to a particular UART driver without a separate driver
experiment.

#### Shared state introduced by Task C

| Variable | Writers | Readers |
|---|---|---|
| `pressure_mv` | `task_sampling()` in `sampling_thread` | `task_control()`, telemetry, display, `status` in `main` |
| `estop` | `task_sampling()`, `joystick_update()` in sampling thread; `clear`/`JOY_SEL` paths in `main` | `task_control()`, telemetry, display, `status` |
| `setpoint_mv` | Console `set` in `main`; `joystick_update()` in sampling thread | `task_control()`, telemetry, display, `status` |

This laboratory code gets away with these accesses only provisionally: the
single-core C6 performs naturally aligned word-sized reads/writes and the
scenario tolerates an occasionally stale value. It is not a synchronization
proof; mutexes, atomic protocols, and priority inversion are addressed later
in the course.

#### Week 3 verdict

Task C successfully demonstrates that the kernel thread protects sampling from
the long console computation. It does **not** prove the hard sampling
requirement: 1.0020 ms is 2 us later than a strict 1 ms deadline. It also does
not protect control, which still belongs to the blocked main loop. The separate
C6 pulse-width capture records `C_obs,max = 2.7 us`; that observed execution
width must not be confused with the 11 us release-to-thread-start latency.

### 3.5 Week 4 — Full kernel migration and A/B

The Week-4 implementation completes the mapping from superloop work to Zephyr
kernel objects. `sampling_tid` (priority 2) consumes `tick_q`; every tenth
sample gives `control_sem`, waking `control_tid` (priority 3). Flow ISR work is
deferred into the private `flow_wq` at priority 5. Display, telemetry, and
console run as sleeping threads at priorities 7, 8, and 9 respectively. The
lower numeric priority is higher in Zephyr's preemptive range.

#### A/B result

| Measurement | C6 superloop, Week 3 | C6 full kernel, Week 4 | Change |
|---|---:|---:|---:|
| Max sampling jitter, 30 s idle | 105 µs | 6 µs | 99 µs less observed jitter |
| Max sampling jitter, `calib` | 402.47 ms | 6.2 µs | removes ~402 ms of observed interference |
| Max control period, `calib` | 412.47 ms | 10.003 ms | removes ~402 ms of observed interference |
| `lat_peak_us`, ISR -> sampling start | — | 10 µs | new measured hand-off metric |

The kernel values use the same board, instrumentation GPIOs and command load as
the C6 Week-3 comparison. The idle capture used 30 M samples at 1 MHz for 30 s;
the `calib` capture used 30 M samples at 5 MHz for 6 s. See EV-W04-AB-001 for
the raw screenshots, min/max values and reproducible commands.

The full migration is an architectural success compared with the superloop,
but it remains an observed result: sampling's 1.0056 ms maximum under `calib`
is 5.6 µs late against a strict 1 ms deadline, and control's 10.003 ms maximum
is 3 µs late against 10 ms. Neither result is a WCET bound.

#### Cost and stack result

Five cursor measurements at 10 MHz gave a maximum visible DIO0-fall to
DIO1-rise gap of 11.7 µs. At 100 control releases per second this is
`100 × 11.7 µs = 1.17 ms/s`, or 0.117 % of a second. This includes semaphore,
scheduler and thread-start effects, rather than only register save/restore.

Stack analysis ran while AD2 generated 100 Hz flow pulses into GPIO3 and while
the console executed `calib`. The closest stack was the system `idle` thread at
228/256 B; `control_tid` used 236/1024 B. The optional SSD1306 was unavailable,
so no full I2C display redraw was included in this particular high-water run.
See EV-W04-COST-002 and EV-W04-STACK-003.

### 3.6 Week 5 — Tracing and instrumentation cost

#### Predictions and measurements

The predictions below were stated before taking the Week-5 measurements. CPU
share was descriptive, so the guide did not request a prediction for that row.

| Measurement | Prediction | Observed value | Contrast |
|---|---:|---:|---|
| Context switch in CTF trace | 15 µs | 4.837 µs | Week-4 GPIO hand-off: 11.7 µs on ESP32-C6; different board and transition |
| Tick ISR to sampling start in CTF trace | 12 µs | 20.012 µs | Firmware `lat_peak_us`: 30 µs; cumulative peak, not necessarily this tick |
| Per-thread CPU share in normal operation | Not requested | `idle` 97.99 %, `sampling_tid` 0.94 %, `tracing_thread` 0.79 %, other threads as below | Shares derived from analyzer cycles and sum to 100 % |
| Increase in maximum sampling jitter with tracing | 10 µs | 10.4 µs | AD2: 12.7 µs tracing on minus 2.3 µs tracing off |

Week 5 used the ESP32-S3 DevKitC. The CTF event log shows a tick interrupt at
130.925 µs, `sampling_tid` switched in at 150.937 µs, then the sampling thread
blocked and `console_tid` resumed. The observed ISR-to-sampling-start interval
was 20.012 µs; the firmware's cumulative `lat_peak_us` was 30 µs. The observed
`console_tid`-to-`sampling_tid` switch interval in that trace was 4.837 µs. These
are CTF timestamps and include tracing effects; Week 4's 11.7 µs GPIO hand-off
was measured on the ESP32-C6, so the values are not a same-board instrument A/B.

The chain matches the lab's expected sequence: tick ISR → `sampling_tid` ready
(the ISR's `k_msgq_put`) → `console_tid` switched out with no `thread_sched_pend`,
so it was preempted while still ready → `sampling_tid` runs and `instr_samp`
(GPIO4, `pins=16`) rises → `sampling_tid` pends in `k_msgq_get` → `console_tid`
resumes its `k_busy_wait(400)`.

At the tenth tick, `k_sem_give` does **not** send the CPU to control. The give
occurred at 12.171 ms and `control_tid` became ready at 12.175 ms. Sampling
(priority 2) kept the CPU until it pended at 12.184 ms, and only then was
`control_tid` (priority 3) switched in, at 12.195 ms, raising `instr_ctrl`
(GPIO5, `pins=32`) at 12.202 ms. The annotated event logs are
`EV-W05-PREEMPT-002-annotated.png` and `EV-W05-CONTROL-003-annotated.png`.

#### Why the trace and the GPIO measurements differ

The two instruments do not time the same thing, so their numbers are not
expected to match:

1. **Different observation points.** CTF timestamps are taken in software at
   kernel hooks (`isr_enter`, `thread_switched_out/in`). GPIO edges come from
   `instr_set()` calls inside the task code. The CTF "switch" of 4.837 µs is
   only the gap between the switched-out and switched-in hooks within one
   switch. The Week-4 11.7 µs GPIO gap runs from the end of sampling's code to
   the start of control's code, and therefore also includes `k_sem_give`, the
   pend, the scheduler decision and the return from `k_sem_take`.
2. **Different transitions.** The 4.837 µs trace value is a console →
   sampling preemption; the 11.7 µs GPIO value is a sampling → control
   semaphore hand-off.
3. **The tracer adds time to what it observes.** Every traced call costs time
   on the S3: one traced `gpio_port_set_bits_raw` took 154.654 → 157.287 µs,
   about 2.6 µs from enter to exit. The ISR → sampling-start path of 20.012 µs
   contains three recorded events, so part of it is the tracer itself. This is
   consistent with the 10.4 µs jitter increase measured by the AD2, which does
   not perturb the target.
4. **Different statistics.** The 20.012 µs value is one tick. `lat_peak_us`
   (30 µs) is the firmware's cumulative maximum since boot, taken with tracing
   enabled, so it need not correspond to the traced tick.
5. **Different boards.** The Week-4 GPIO reference was measured on the
   ESP32-C6; the Week-5 trace was taken on the ESP32-S3.

For these reasons the AD2 remains the judge of the numbers (the period and
jitter of EV-W05-JITTER-ON-004/OFF-005), while the CTF trace is used for the
story: who ran, in what order, and why.

The AD2 measured a nominal 1 ms sampling period with tracing enabled and
disabled under the same 30 M sample, 10 MHz protocol. Define maximum jitter here
as the greatest absolute deviation of the observed period from 1 ms:

| Condition | Mean period | Minimum | Maximum | Maximum absolute jitter |
|---|---:|---:|---:|---:|
| CTF tracing on | 0.99999 ms | 0.98730 ms | 1.0065 ms | 12.7 µs |
| CTF tracing off | 1.00000 ms | 0.99770 ms | 1.0021 ms | 2.3 µs |

The observed increase in maximum absolute jitter with tracing was **10.4 µs**.
This is the measured observer cost for this S3 run, not a guaranteed constant.

For normal-operation CPU share, the `threads` analyzer output included both
`idle` and `tracing_thread`. Its printed whole-number CPU percentages truncated
small shares to zero, so shares were calculated from the reported per-thread
cycle counts and normalized by their total. This analyzer snapshot is cumulative
since boot. `CONFIG_TRACING_IDLE=n` remains necessary in `trace.conf` to keep the
UART CTF stream within bandwidth; therefore the Gantt viewer alone cannot provide
the complete idle-inclusive CPU distribution.

The calibration trace contains only the first approximately 75 ms of the
`calib` busy-wait loop because the 32 KiB CTF buffer fills while the low-priority
`tracing_thread` cannot drain it: the higher-priority console thread spins in
`k_busy_wait(400)` instead of yielding. The text export contains 189 busy-wait
enter/exit pairs; 189 × 400 µs = 75.6 ms, consistent with the buffer-limited
portion that survives. The command itself continues for about 400 ms.

#### Week 5 evidence

- `EV-W05-CPU-001.md`: analyzer transcript, cycle counts, and normalized shares.
- `EV-W05-PREEMPT-002.md`, `.png` and `-annotated.png`: preemption trace during `calib`.
- `EV-W05-CONTROL-003.png` and `-annotated.png`: semaphore release and control-thread dispatch.
- `EV-W05-JITTER-ON-004.md` and `EV-W05-JITTER-OFF-005.md` with their `.png`:
  paired AD2 captures. The firmware commit and the workload still need to be
  confirmed (OI-007).

### 3.7 Week 6 — Schedulability, response time and priority inversion

#### Predictions (stated before the lab)

Predicted values were derived from the task parameters and earlier
measurements before running the Week-6 experiments. Measured values are filled
in as each task is completed.

| Measurement | Prediction | Basis of the prediction | Observed value | Reference |
|---|---|---|---:|---|
| Fig. 4.13 under RM: who misses, when? | τ2 misses at t = 7 ms | RTA: `R2 = 4 + ⌈R2/5⌉·2` converges to 8 ms > `D2` = 7 ms | ____ | Book: τ2 at t = 7 ms |
| Fig. 4.13 under EDF: misses in 60 s | 0 | `U = 2/5 + 4/7 ≈ 0.971 ≤ 1` | ____ | Book: none |
| Control's max R, no lock contention | ≈ 12 µs | Hand-off 11.7 µs (EV-W04-COST-002) + `C_obs,max` 0.7 µs (EV-W02-TASKS-006) | ____ | RTA script |
| Control's max R during the inversion (`k_sem` as lock) | Unbounded by the analysis; grows with all medium-priority telemetry work | Medium thread preempts the lock holder | ____ | — |
| Control's max R with a `k_mutex` | ≈ `C + B + interference` | Priority inheritance bounds blocking to one critical section | ____ | ≤ C + B + interference |
| B: the low thread's longest critical section | ____ | To be measured by its GPIO pulse | ____ | GPIO pulse |
| `calib` CPU / duration, before the fix | ≈ 400 ms / 400 ms | 1000 × `k_busy_wait(400)`: spinning, so CPU = duration | ____ | CPU = duration |
| `calib` CPU / duration, after the fix | A few ms / ≈ 500 ms | `k_usleep(400)` rounds up to the 100 µs tick; CPU is only the reads | ____ | — |
| E-stop, detection → valve low, max of 5 trips, before / after | ≈ 11 ms / < 1 ms | Before: up to one control period (10 ms) + one PWM step (1 ms). After: valve driven low in the same sampling job | ____ | Sum of the path in Task C |

### 3.8 Future-week evidence rule

Each new measurement record must state:

1. objective and linked requirements;
2. board and serial/nickname;
3. firmware Git commit and configuration;
4. workload and environmental conditions;
5. instrument, resolution, and connections;
6. observation interval and sample count;
7. min/mean/p99/max or another justified statistic;
8. explicit `PASS`, `FAIL`, or `INCONCLUSIVE` verdict and deadline margin;
9. paths to raw data, scripts, and figures.

## 4. Schedulability analysis

### 4.1 Input data

All `C_i` inputs shall cite measured evidence and the configuration under which
they were obtained.

| Task | `C_i` evidence | `C_i` used | `T_i` | `D_i` | `B_i` | Notes |
|---|---|---:|---:|---:|---:|---|
| Sampling + safety check | EV-W02-BASELINE-002 (NUCLEO, 3 µs); EV-W03-CI-008 (C6, 2.7 µs) | TBD — reference platform pending (OI-008) | 1 ms | 1 ms | 0 (no mutexes) | GPIO + analyzer |
| Control loop | EV-W02-TASKS-006 (NUCLEO, 0.7 µs) | TBD — reference platform pending (OI-008) | 10 ms | 10 ms | TBD | GPIO + analyzer; C6/S3 width not yet measured |
| Command console | EV-W02-TASKS-006 (NUCLEO, `status` 6.6010 ms); EV-W02-BLOCK-005 (`calib` ≈ 414 ms) | TBD | Sporadic | TBD | TBD | GPIO + analyzer; firm deadline open (OI-004) |
| Telemetry | EV-W02-TASKS-006 (NUCLEO, 5.7770 ms) | TBD — reference platform pending (OI-008) | 1 s | 1 s (derived from task period) | TBD | GPIO + analyzer |
| Display HMI | Not measured | TBD | 500 ms | TBD | TBD | Optional task; not enabled in measured runs |

### 4.2 Tests and results

| Analysis | Applicability | Result | Evidence/tool | Verdict |
|---|---|---|---|---|
| Utilization `U = sum(C_i/T_i)` | Periodic task subset | TBD | Pending week 6 | PENDING |
| Liu & Layland RM bound | Fixed-priority periodic tasks | TBD | Pending week 6 | PENDING |
| Hyperbolic test | Fixed-priority periodic tasks | TBD | Pending week 6 | PENDING |
| EDF utilization test | Independent periodic tasks under EDF | TBD | Pending week 6 | PENDING |
| Response-Time Analysis | Per fixed-priority task | TBD | Pending week 7 | PENDING |
| Extended RTA with blocking | Tasks sharing mutexes | TBD | Pending week 7 | PENDING |
| CBS bandwidth | Hub `SCHED_DEADLINE` task | TBD | Pending week 11 | PENDING |

### 4.3 Per-task verdict

| Task | `R_i` | `D_i` | Margin `D_i - R_i` | Analytical verdict | Measured verdict |
|---|---:|---:|---:|---|---|
| Sampling + safety check | Not yet bounded analytically | 1 ms | TBD | PENDING | FAIL strict — C6 full kernel maximum 1.0056 ms during `calib`; major mitigation versus 403.47 ms superloop |
| Control loop | TBD | 10 ms | TBD | PENDING | FAIL strict — C6 full kernel maximum 10.003 ms during `calib`; major mitigation versus 412.47 ms superloop |

## 5. Functional safety

### 5.1 Safe state

- **Declared safe state:** valve closed and pump de-energized.
- **Trigger conditions:** TBD during the final-project hazard analysis.
- **Maximum time to safe state:** TBD requirement.

### 5.2 Watchdog chain

| Item | Decision | Evidence |
|---|---|---|
| Who kicks the watchdog | TBD | Pending project design |
| What condition stops the kick | TBD | Pending project design |
| Who/what bites | TBD | Pending project design |
| Physical action on bite | Valve closes; implementation TBD | Pending checkpoint 2 |
| Measured fail-safe time | TBD | Pending checkpoint 2 |

## Appendix A — Evidence naming and verdicts

- Evidence IDs use `EV-WNN-TYPE-NNN`, for example `EV-W04-AB-001`.
- Store the evidence record under `evidence/weekNN/`.
- Preserve raw data; processed plots must cite both the raw file and generating
  command/script.
- `PASS`: requirement met with a reported positive margin.
- `FAIL`: requirement missed or margin is negative.
- `INCONCLUSIVE`: protocol or data cannot support a verdict.
- `PENDING`: verification has not yet been performed.

## Appendix B — Open items

| ID | Item | Owner | Due | Status |
|---|---|---|---|---|
| OI-001 | Confirm course-provided baseline board for weeks 1–2; student bring-up uses NUCLEO-G474RE | Team/instructor | Before week 2 | OPEN — overdue; weeks 1–2 were run on the NUCLEO-G474RE as a declared deviation from the course NUCLEO-L476RG |
| OI-002 | Record team member names | Team | Week 1 | CLOSED — Juan Pablo Vargas Cordoba |
| OI-003 | Record board model, serial, and nickname | Team | Week 1 | CLOSED — NUCLEO-G474RE and STLINK-V3 identifier recorded |
| OI-004 | Define numeric jitter and firm/soft deadlines | Team/instructor | Before verification | PARTIAL — sampling deadline fixed at 1 ms; console firm deadline remains TBD |
| OI-005 | Declare/install the editor used for the course | Team | Before week 1 | CLOSED — GNU nano 7.2 used |
| OI-006 | Record the ESP32-S3 DevKitC serial/nickname used in Week 5 | Team | Week 6 | OPEN |
| OI-007 | Confirm firmware commit and workload in EV-W05-JITTER-ON-004 and EV-W05-JITTER-OFF-005 | Team | Week 6 | PARTIAL — records written from the captures; commit and workload unconfirmed |
| OI-008 | Fix the reference platform for §4 schedulability inputs and measure every `C_i` on it | Team | Week 6 | OPEN |
