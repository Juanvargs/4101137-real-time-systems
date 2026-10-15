# EV-W03-BASELINE-002 — ESP32-C6 DevKitC, superloop sin hilo

## Objetivo

Caracterizar el período de `task_sampling()` del superloop en la ESP32-C6
DevKitC antes de introducir el hilo de muestreo. Vincula REQ-CTRL-03.

## Plataforma y configuración

- Placa: ESP32-C6 DevKitC compatible, módulo de flash de 4 MB.
- Target Zephyr: `esp32c6_devkitc/esp32c6/hpcore`.
- Firmware: `firmware/superloop` en arquitectura superloop (sin el cambio de
  Task C); estado de trabajo de Week 3 previo al commit.
- Instrumento: Analog Discovery 2, WaveForms 3.25.1.
- Conexión: DIO0 -> GPIO18 (`instr_samp`); GND común.
- Captura: Logic / Record, 30 M muestras, 1 MHz, intervalo de 30 s, sin
  comando de consola.

## Resultado

La captura preservada es [EV-W03-BASELINE-002.png](EV-W03-BASELINE-002.png).

| Señal | Conteo | Período medio | Mínimo | Máximo | Resultado derivado |
|---|---:|---:|---:|---:|---:|
| DIO0, `instr_samp` | 29,999 | 1.00000 ms | 0.89700 ms | 1.1050 ms | `J_obs,max = 105 us` |
| DIO3, telemetría | 29 | 1.00000 s | 0.99999 s | 1.0000 s | Cadencia de 1 Hz observada |

La máxima desviación absoluta respecto al período nominal de 1 ms es
`max(1.1050 - 1.0000, 1.0000 - 0.8970) ms = 105 us`.

La fila DIO1 de esta captura no se usa para medir el período de control: a
1 MHz sus pulsos cortos no se detectaron de forma fiable. El período de control
se repitió a 5 MHz en EV-W03-CALIB-003.

## Veredicto

**FAIL estricto para REQ-CTRL-03.** Se observó un período de 1.1050 ms, 105 us
por encima de la separación nominal de 1 ms. Este resultado es una observación,
no un límite WCET.
