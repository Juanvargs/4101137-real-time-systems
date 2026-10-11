# EV-W02-TASKS-006 — Remaining instrumented task widths

## Identification

| Field | Value |
|---|---|
| Date | 2026-10-10 |
| Operator | Juan Pablo Vargas Cordoba |
| Objective | Complete observed execution-width records for control, normal console, telemetry, and flow batch |
| Linked requirements | REQ-CTRL-01, REQ-CTRL-04, REQ-CTRL-05 |
| Platform | NUCLEO-G474RE (`nucleo_g474re/stm32g474xx`), cache ON |
| Instrument | Analog Discovery 2; WaveForms 3.25.1 Logic Analyzer |
| Wiring | DIO1→D4/PB5, DIO2→D5/PB4, DIO3→D6/PB10, DIO4→D7/PA8; common GND |
| Verdict | CHARACTERIZED — widths recorded; this record does not establish WCET or unresolved requirement verdicts |

## Protocol

- Control: 30,000,000 samples at 10 MHz for 3 s (0.1 us resolution), no console command and no Pattern stimulus.
- Normal console: 30,000,000 samples at 1 MHz for 30 s (1 us resolution); one `status` command through ST-LINK VCP at 115200 8N1.
- Telemetry: 30,000,000 samples at 1 MHz for 30 s (1 us resolution), after reset with no console command.
- Flow batch: same 30 s and 1 MHz configuration; AD2 DIO6 drove Nucleo D9/PC7 at 100 Hz, 50% duty, push-pull, idle low. DIO4 recorded one batch per 100 input pulses.

## Results

| Signal / task | Count | Mean positive width | Minimum | Maximum observed |
|---|---:|---:|---:|---:|
| DIO1 / control | 300 | 0.63300 us | 0.60000 us | 0.70000 us |
| DIO2 / console, one `status` | 8 | 0.85037 ms | 4 us | 6.6010 ms |
| DIO3 / telemetry | 30 | 5.7682 ms | 5.6900 ms | 5.7770 ms |
| DIO4 / flow batch | 30 | 34.033 us | 33 us | 44 us |

Control's count agrees with its intended 10 ms interval over the 3 s capture.
Console's eight pulses correspond to received command bytes; its mean combines
short character handling with the longer end-of-line `status` handler. The
maximum is therefore a workload-specific observed value, not a console WCET.
Telemetry's roughly 5.8 ms width helps explain why a non-preemptive superloop
can delay 1 kHz sampling. Flow's 30 batches agree with the 100 Hz stimulus and
one batch per 100 pulses.

## Artifacts

- [Control, 10 MHz](EV-W02-TASKS-006-control.png)
- [Console `status`](EV-W02-TASKS-006-console.png)
- [Telemetry](EV-W02-TASKS-006-telemetry.png)
- [Flow batch](EV-W02-TASKS-006-flow.png)
