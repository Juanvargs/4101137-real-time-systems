# RET — Timing Evidence Report

## Document control

| Field | Value |
|---|---|
| Document ID | RET-STR-001 |
| Project | SoilSense Control / SoilSense Hub |
| Course | Real-Time Systems — UNAL 4101137 |
| Team | Individual — Juan Pablo Vargas Cordoba |
| Authors | Juan Pablo Vargas Cordoba |
| Status | Draft — Week 1 bring-up and initial task set |
| Version | 0.2 |
| Date | 2026-09-21 |
| Repository baseline | `cdc0471` |
| Starter board | NUCLEO-G474RE — student-owned; course baseline differs |
| Board/debug identifier | STLINK-V3 `003400303232510139353236` |
| Production MCU | ESP32-S3 (from week 3) |

### Revision history

| Version | Date | Author | Change |
|---|---|---|---|
| 0.1 | 2026-09-20 | Juan Pablo Vargas Cordoba | Professional RET structure and first environment evidence |
| 0.2 | 2026-09-21 | Juan Pablo Vargas Cordoba | Week 1 hardware evidence and scenario-derived initial task set |

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
| REQ-CTRL-03 | While control is enabled, the system shall sample the pressure input at 1 kHz with a maximum release jitter of `TBD`. | Hard | Incomplete | GPIO + logic analyzer |
| REQ-CTRL-04 | When a telemetry job is released, the system shall publish the current state within `TBD`. | Soft | Incomplete | Trace and output timestamp |
| REQ-CTRL-05 | When a valid console command is received, the system shall produce a usable response within `TBD`; a response after that limit may be discarded. | Firm | Incomplete | UART timestamps + trace |

### 1.3 Task set

`C_i` is the execution time of task `i`. It remains `____` until it is measured
under a declared protocol. A future maximum observed value will be reported as
`C_obs,max`; it shall not be called WCET without a justified upper-bound method.

| Task | Requirement | Type | Activation/period | Deadline | Measured `C_i` | Jitter bound | Priority/policy | Measurement method |
|---|---|---|---:|---:|---:|---:|---|---|
| Sensor sampling | REQ-CTRL-03 | Hard | Periodic, 1 ms (1 kHz) | TBD | ____ | TBD | TBD | GPIO + analyzer |
| Control loop | REQ-CTRL-01 | Hard | Periodic, 10 ms | 10 ms | ____ | TBD | TBD | GPIO + analyzer |
| Overpressure emergency stop | REQ-CTRL-02 | Hard | Event-triggered | < 5 ms | ____ | N/A | TBD | Instrumented input-to-output latency |
| Command console | REQ-CTRL-05 | Firm | Event-triggered/sporadic | TBD | ____ | N/A | TBD | UART timestamps + trace |
| Telemetry | REQ-CTRL-04 | Soft | TBD | TBD | ____ | TBD | TBD | Output timestamps + trace |

No execution time, telemetry period, console deadline, sensor deadline, or jitter
bound has been invented; each unresolved value remains explicit until the course
provides it or the team measures and justifies it.

### 1.4 Initial traceability matrix

| Requirement | Implementing task | Analysis | Evidence | Verdict |
|---|---|---|---|---|
| REQ-CTRL-01 | Control loop | Utilization + RTA | Pending week 2 | PENDING |
| REQ-CTRL-02 | Overpressure emergency stop | Input-to-output response bound | Pending measurement | PENDING |
| REQ-CTRL-03 | Sensor sampling | Period/jitter analysis | Pending measurement | PENDING |
| REQ-CTRL-04 | Telemetry | Response-time characterization | Pending week 2 | PENDING |
| REQ-CTRL-05 | Command console | Response-time characterization | Pending week 2 | PENDING |

## 2. Architecture Decision Records

No architecture decision is accepted before its alternatives and timing evidence
exist.

### ADR-001 — Control-node execution architecture

- **Status:** Not started; decision due in week 4.
- **Context:** Compare the superloop and multithreaded Zephyr implementation on the
  same ESP32-S3, with the same task set and load.
- **Alternatives:** superloop; multithreaded kernel; hybrid architecture.
- **Decision:** TBD.
- **Quantitative justification:** Pending week-2 to week-4 A/B evidence.
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

### 3.3 Week 2 onward

Add one subsection per week. Each measurement record must state:

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
| Sampling + safety check | TBD | TBD | 1 ms | TBD | TBD | Pending week 2 |
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
| Sampling + safety check | TBD | TBD | TBD | PENDING | PENDING |
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
| OI-002 | Record team member names | Team | Week 1 | OPEN |
| OI-003 | Record board model, serial, and nickname | Team | Week 1 | OPEN |
| OI-004 | Define numeric jitter and firm/soft deadlines | Team/instructor | Before verification | OPEN |
| OI-005 | Declare/install the editor used for the course | Team | Before week 1 | OPEN |
