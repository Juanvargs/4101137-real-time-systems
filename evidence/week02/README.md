# Week 2 — Superloop evidence index

| ID | Topic | Main result |
|---|---|---|
| EV-W02-FLOW-001 | Architecture map | Timer and flow ISRs hand off work to one main loop. |
| EV-W02-BASELINE-002 | Cache-on baseline | Sampling mean 1.0001 ms, but maximum period 6.845 ms; REQ-CTRL-03 fails. |
| EV-W02-FLOWLAT-003 | Flow ISR hand-off | One observed ISR-to-loop service latency was approximately 6 us. |
| EV-W02-CACHE-004 | Flash accelerator off | Sampling width increased about 36.2%; maximum jitter changed from 5.845 to 5.857 ms. |
| EV-W02-BLOCK-005 | Blocking console command | `calib` raised `backlog_peak` from 6 to 414 and maximum sampling period to 414.06 ms. |
| EV-W02-TASKS-006 | Remaining task widths | Control max 0.7 us; `status` max 6.6010 ms; telemetry max 5.7770 ms; flow max 44 us. |

The supplied PNGs are unchanged screenshots copied from the WaveForms session.
