# EV-W05-PREEMPT-002 — Traza de preempción durante `calib`

**Fecha:** 2026-11-04
**Placa:** ESP32-S3 DevKitC
**Build:** Week 5 con CTF habilitado
**Fuente:** registro de eventos del visor CTF compartido durante la sesión.

## Tick observado mientras corría `calib`

| Tiempo | Evento | Interpretación |
|---:|---|---|
| 130.925 µs | `isr_enter` | Entra el ISR del tick. |
| 138.342 µs | `thread_sched_ready`, `sampling_tid` | El ISR libera al hilo de muestreo. |
| 146.100 µs | `thread_switched_out`, `console_tid` | La consola deja la CPU y queda lista. |
| 150.937 µs | `thread_switched_in`, `sampling_tid` | Empieza el muestreo. |
| 174.533 µs | `thread_sched_pend`, `sampling_tid` | Muestreo se bloquea esperando el siguiente tick. |
| 179.392 µs | `thread_switched_out`, `sampling_tid` | Sale de CPU el hilo de muestreo. |
| 185.679 µs | `thread_switched_in`, `console_tid` | La consola reanuda su `k_busy_wait(400)`. |

El log también muestra los pares `thread_busy_wait_enter` / `thread_busy_wait_exit` con `usec_to_wait=400`, confirmando que el hilo de consola estaba ejecutando `calib`.

## Liberación de control

En el tick de aproximadamente **12 ms** del registro:

| Tiempo | Evento |
|---:|---|
| 12.131 ms | `isr_enter` |
| 12.138 ms | `thread_sched_ready`, `sampling_tid` |
| 12.151 ms | `thread_switched_in`, `sampling_tid` |
| 12.171 ms | `semaphore_give_enter`, id `0x3fc900e4` |
| 12.175 ms | `thread_sched_ready`, `control_tid` |
| 12.184 ms | `thread_sched_pend`, `sampling_tid` |
| 12.189 ms | `thread_switched_out`, `sampling_tid` |
| 12.195 ms | `thread_switched_in`, `control_tid` |
| 12.199 ms | `semaphore_take_exit`, id `0x3fc900e4` |

El `give` y el `take` usan el mismo semáforo. `control_tid` empieza a correr después de que `sampling_tid` se bloquea esperando el siguiente tick.

## Tiempos derivados y comparación

- Cambio observado `console_tid` → `sampling_tid`: 150.937 − 146.100 = **4.837 µs** entre los eventos `thread_switched_out` y `thread_switched_in`.
- Tick ISR → inicio de muestreo: 150.937 − 130.925 = **20.012 µs**.
- `lat_peak_us` leído por consola: **30 µs** (máximo acumulado; no es necesariamente el mismo tick de la traza).
- Referencia Week 4 medida por GPIO: **11.7 µs** en ESP32-C6. La observación CTF anterior es un cambio concreto en ESP32-S3; la comparación es entre instrumentos y placas distintas.

Los timestamps CTF incluyen el costo de instrumentación y no sustituyen la medición por GPIO. La captura de jitter de Week 5, con AD2, permanece como medida externa.

## Evidencia visual

Se conserva el registro de eventos en `EV-W05-PREEMPT-002.png`. En la captura
`EV-W05-CONTROL-003.png`, los eventos clave están en las líneas 242–248:
`semaphore_give_enter`, `control_tid` listo, muestreo bloqueado y `control_tid`
ejecutándose y tomando el semáforo. Las capturas se dejan como están, sin añadir
flechas. Los valores transcritos arriba corresponden a las imágenes compartidas
durante la sesión.
