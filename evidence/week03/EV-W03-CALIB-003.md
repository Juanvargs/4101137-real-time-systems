# EV-W03-CALIB-003 — Interferencia de `calib` en el superloop de ESP32-C6

## Objetivo

Medir cómo el comando bloqueante `calib` afecta a muestreo y control cuando
ambos siguen dentro del superloop. Vincula REQ-CTRL-01, REQ-CTRL-03 y
REQ-CTRL-06.

## Plataforma y protocolo

- ESP32-C6 DevKitC compatible de 4 MB; target
  `esp32c6_devkitc/esp32c6/hpcore`.
- Firmware: superloop anterior al hilo de Week 3.
- Analog Discovery 2 / WaveForms 3.25.1; Logic / Record, 30 M muestras a
  5 MHz (ventana de 6 s).
- DIO0 -> GPIO18 (`instr_samp`) y DIO1 -> GPIO19 (`instr_ctrl`), con GND común.
- Con la captura activa se envió `calib` por la consola a 115200 8N1.

## Resultado

La captura simultánea está preservada en
[EV-W03-SUPERLOOP-CALIB-007.png](EV-W03-SUPERLOOP-CALIB-007.png).

| Señal | Período nominal | Máximo observado | Resultado |
|---|---:|---:|---|
| DIO0, muestreo | 1 ms | 403.47 ms | `J_obs,max = 402.47 ms` |
| DIO1, control | 10 ms | 412.47 ms | La liberación/servicio de control se retrasa durante `calib` |

Un segundo registro, [EV-W03-SUPERLOOP-CONTROL-006.png](EV-W03-SUPERLOOP-CONTROL-006.png),
observó 406.48 ms como máximo en otra repetición. El reporte usa 412.47 ms,
el máximo entre las capturas válidas.

La consola informó `backlog_peak=403`, coherente con aproximadamente 403 ticks
de 1 ms acumulados mientras el trabajo síncrono de calibración ocupa el
superloop.

## Veredicto

**FAIL.** `calib` bloquea a la vez el muestreo y el control en la arquitectura
superloop; el muestreo excede su período nominal por 402.47 ms.
