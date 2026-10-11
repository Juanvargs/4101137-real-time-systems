# RET — Timing Evidence Report

## Document control

| Field | Value |
|---|---|
| Document ID | RET-STR-001 |
| Project | SoilSense Control / SoilSense Hub |
| Course | Real-Time Systems — UNAL 4101137 |
| Team | Individual — Juan Pablo Vargas Cordoba |
| Authors | Juan Pablo Vargas Cordoba |
| Status | Draft — Week 2 superloop evidence complete |
| Version | 0.4 |
| Date | 2026-10-10 |
| Repository baseline | Week 2 evidence update after `1d62d71`; commit pending review |
| Starter board | NUCLEO-G474RE — student-owned; course baseline differs |
| Board/debug identifier | STLINK-V3 `003400303232510139353236` |
| Production MCU | ESP32-S3 (from week 3) |

### Revision history

| Version | Date | Author | Change |
|---|---|---|---|
| 0.1 | 2026-09-20 | Juan Pablo Vargas Cordoba | Professional RET structure and first environment evidence |
| 0.2 | 2026-09-21 | Juan Pablo Vargas Cordoba | Week 1 hardware evidence and scenario-derived initial task set |
| 0.3 | 2026-09-26 | Juan Pablo Vargas Cordoba | Week 2 superloop, flash-accelerator, and blocking-command evidence |
| 0.4 | 2026-10-10 | Juan Pablo Vargas Cordoba | Added observed widths for control, console, telemetry, and flow batch |

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
| REQ-CTRL-01 | While the system is irrigating, the control loop shall be released every 10 ms and shall complete no later than its next release. | Hard | Draft | GPIO + logic analyzer; RTA |
| REQ-CTRL-02 | When measured pressure exceeds the safety threshold, the system shall command the valve/pump to its safe state within 5 ms. | Hard | Draft | Instrumented input-to-output latency |
| REQ-CTRL-03 | While control is enabled, the system shall sample the pressure input every 1 ms and shall complete each sampling job before its next release. | Hard | Measured — FAIL in current superloop | GPIO + logic analyzer |
| REQ-CTRL-04 | While the control node is operational, the system shall publish telemetry of its current state at least once every 1000 ms. | Soft | Observed; detailed deadline analysis pending | GPIO + logic analyzer |
| REQ-CTRL-05 | When a valid console command is received, the system shall produce a usable response within `TBD`; a response after that limit may be discarded. | Firm | Incomplete | UART timestamps + trace |
| REQ-CTRL-06 | While a firm console command is executing, the system shall continue to satisfy all hard sampling deadlines. | Hard | Measured — FAIL in current superloop | GPIO + logic analyzer + serial status |

### 1.3 Task set

`C_i` is the execution time of task `i`. It remains `____` until it is measured
under a declared protocol. A future maximum observed value will be reported as
`C_obs,max`; it shall not be called WCET without a justified upper-bound method.

| Task | Requirement | Type | Activation/period | Deadline | Measured `C_i` | Jitter bound | Priority/policy | Measurement method |
|---|---|---|---:|---:|---:|---:|---|---|
| Sensor sampling | REQ-CTRL-03 | Hard | Periodic, 1 ms (1 kHz) | 1 ms | `C_obs,max` 3 us baseline; 4 us cache-off; 12 us during `calib` | 5.845 ms normal; 413.06 ms during `calib` | Single superloop | GPIO + analyzer |
| Control loop | REQ-CTRL-01 | Hard | Periodic, 10 ms | 10 ms | `C_obs,max` 0.7 us (300 runs, 10 MHz capture) | TBD | Single superloop | GPIO + analyzer |
| Overpressure emergency stop | REQ-CTRL-02 | Hard | Event-triggered | < 5 ms | Shared with sampling; standalone response not measured | N/A | TBD | Instrumented input-to-output latency |
| Command console | REQ-CTRL-05, REQ-CTRL-06 | Firm | Event-triggered/sporadic | TBD | `status` path: 4 us to 6.6010 ms across 8 received bytes; `calib` blocks the loop for approximately 414 ms; neither is a WCET | N/A | Single superloop | UART + DIO2 trace |
| Telemetry | REQ-CTRL-04 | Soft | Periodic, 1 s | 1 s (derived from task period) | `C_obs,max` 5.7770 ms (30 runs) | TBD | Single superloop | DIO3 trace |
| Flow batch | N/A | N/A | Every 100 flow pulses | TBD | `C_obs,max` 44 us (30 batches at 100 Hz stimulus) | N/A | Single superloop | DIO4 trace |
| Display HMI | N/A | Soft | Periodic, 500 ms when enabled | TBD | Not enabled on this run | N/A | Single superloop | DIO5 trace |

No unmeasured execution time, telemetry period, console deadline, sensor deadline,
or jitter bound has been invented; each unresolved value remains explicit until the
course provides it or the team measures and justifies it.

### 1.4 Initial traceability matrix

| Requirement | Implementing task | Analysis | Evidence | Verdict |
|---|---|---|---|---|
| REQ-CTRL-01 | Control loop | Utilization + RTA | EV-W02-TASKS-006 records observed width; end-to-end control verification is pending | INCONCLUSIVE |
| REQ-CTRL-02 | Overpressure emergency stop | Input-to-output response bound | Pending measurement | PENDING |
| REQ-CTRL-03 | Sensor sampling | Period/jitter analysis | EV-W02-BASELINE-002, EV-W02-CACHE-004, EV-W02-BLOCK-005 | FAIL |
| REQ-CTRL-04 | Telemetry | Period characterization | EV-W02-TASKS-006 observed 30 publications in 30 s; period statistic pending | INCONCLUSIVE |
| REQ-CTRL-05 | Command console | Response-time characterization | EV-W02-TASKS-006 (`status`) and EV-W02-BLOCK-005; deadline is TBD | INCONCLUSIVE |
| REQ-CTRL-06 | Console versus hard sampling | Non-interference trace | EV-W02-BLOCK-005 | FAIL |

## 2. Architecture Decision Records

No architecture decision is accepted before its alternatives and timing evidence
exist.

### ADR-001 — Control-node execution architecture

- **Status:** Not started; decision due in week 4.
- **Context:** Compare the superloop and multithreaded Zephyr implementation on the
  same ESP32-S3, with the same task set and load.
- **Alternatives:** superloop; multithreaded kernel; hybrid architecture.
- **Decision:** TBD.
- **Quantitative justification:** Week 2 measured a 6.845 ms maximum sampling
  period against a 1 ms deadline in normal operation and 414.06 ms during the
  blocking console command. The kernel comparison remains due in weeks 3–4.
- **Consequences and costs:** TBD.

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
| VS Code or the selected editor is available | PENDING | `code` not found; alternative editor not yet declared |
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

### 3.4 Future-week evidence rule

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
| Sampling + safety check | EV-W02-BASELINE-002 | `C_obs,max` 3 us (cache-on baseline) | 1 ms | 1 ms | 0 (no mutexes) | Measured period still fails due to loop interference |
| Control loop | TBD | TBD | 10 ms | 10 ms | TBD | Pending week 2 |
| Command console | TBD | TBD | Sporadic | TBD | TBD | Pending week 2 |
| Telemetry | TBD | TBD | 1 s | TBD | TBD | Period provisional |
| Display HMI | TBD | TBD | 500 ms | TBD | TBD | Optional task |

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
| Sampling + safety check | Not yet bounded analytically | 1 ms | TBD | PENDING | FAIL — max period 6.845 ms baseline; 414.06 ms during `calib` |
| Control loop | TBD | 10 ms | TBD | PENDING | PENDING |

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
| OI-001 | Confirm course-provided baseline board for weeks 1–2; student bring-up uses NUCLEO-G474RE | Team/instructor | Before week 2 | OPEN |
| OI-002 | Record team member names | Team | Week 1 | CLOSED — Juan Pablo Vargas Cordoba |
| OI-003 | Record board model, serial, and nickname | Team | Week 1 | CLOSED — NUCLEO-G474RE and STLINK-V3 identifier recorded |
| OI-004 | Define numeric jitter and firm/soft deadlines | Team/instructor | Before verification | PARTIAL — sampling deadline fixed at 1 ms; console firm deadline remains TBD |
| OI-005 | Declare/install the editor used for the course | Team | Before week 1 | CLOSED — GNU nano 7.2 used |
