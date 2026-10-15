# EV-W03-THREAD-004 — Hilo de muestreo en la ESP32-C6

## Objetivo

Verificar Task C: mover únicamente el muestreo a un `k_thread` preemptivo que
recibe timestamps desde una `k_msgq`, manteniendo el control y el resto de
actividades en el superloop de `main`.

## Implementación verificada

- `CONFIG_MAIN_THREAD_PRIORITY=10` baja la prioridad del hilo `main`.
- La ISR periódica encola el instante de liberación en `tick_q` sin bloquear.
- `sampling_thread`, prioridad preemptiva 2, espera la cola, mide
  `lat_peak_us`, ejecuta `task_sampling()` y publica cada décima liberación en
  `control_pending`.
- `main` consume `control_pending` para ejecutar `task_control()`; por eso
  `calib` todavía puede atrasar el control.

## Captura con `calib`

- Placa/target: ESP32-C6 DevKitC compatible de 4 MB /
  `esp32c6_devkitc/esp32c6/hpcore`.
- Instrumento: Analog Discovery 2, WaveForms 3.25.1, Logic / Record, 30 M
  muestras a 5 MHz (6 s).
- DIO0 -> GPIO18 (`instr_samp`); DIO1 -> GPIO19 (`instr_ctrl`); GND común.
- Carga: se envió `calib` durante la captura.

La evidencia simultánea es
[EV-W03-THREAD-CALIB-004.png](EV-W03-THREAD-CALIB-004.png).

| Señal | Período nominal | Máximo observado | Interpretación |
|---|---:|---:|---|
| DIO0, muestreo | 1 ms | 1.0020 ms | `J_obs,max = 2 us` durante `calib` |
| DIO1, control | 10 ms | 412.31 ms | Sigue bloqueado por `calib` en `main` |

La consola posterior a la prueba reportó `backlog_peak=0` y
`lat_peak_us=11`. La cola no acumuló ticks porque el hilo de prioridad 2
preempta al `main` de prioridad 10.

## Reposo de 30 s

A 5 MHz, 30 M muestras sólo cubren 6 s. Se hicieron cinco capturas consecutivas
de 6 s para obtener 30 s totales. Los máximos de DIO0 fueron 1.0018, 1.0016,
1.0020, 1.0020 y 1.0020 ms. El máximo global fue 1.0020 ms (`J_obs,max = 2 us`).
La quinta captura se preserva en
[EV-W03-THREAD-IDLE-005.png](EV-W03-THREAD-IDLE-005.png).

## Veredicto

El hilo evita que `calib` destruya el muestreo: de 403.47 ms a 1.0020 ms de
período máximo observado. Sin embargo, para el deadline estricto de 1 ms el
máximo medido queda 2 us tarde; es una gran mejora experimental, no una prueba
formal de cumplimiento hard real-time. El control aún falla bajo `calib` porque
no se movió fuera de `main`.
