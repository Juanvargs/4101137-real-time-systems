# EV-W04-COST-002 — Coste visible del cambio de contexto

## Objetivo

Completar Task C: medir el intervalo visible desde que `sampling_thread`
termina `task_sampling()` hasta que `control_thread` entra en
`task_control()` cada décima muestra.

## Protocolo

- Placa/target: ESP32-C6 DevKitC compatible de 4 MB /
  `esp32c6_devkitc/esp32c6/hpcore`.
- Firmware: migración completa de Week 4 en `firmware/superloop/`.
- Instrumento: Analog Discovery 2, WaveForms 3.25.1, Logic / Record,
  30 M muestras a 10 MHz: ventana de 3 s y resolución de 0.1 us.
- Conexiones: DIO0 -> GPIO18 (`instr_samp`), DIO1 -> GPIO19 (`instr_ctrl`) y
  GND común.
- Carga: ninguna orden de consola durante la captura.
- Cursores: X1 sobre el flanco de bajada de DIO0 y X2 sobre el flanco de subida
  inmediatamente posterior de DIO1.

## Muestras observadas

| Captura | `Delta X` |
|---|---:|
| [CONTEXT-003](EV-W04-CONTEXT-003.png) | 11.1 us |
| [CONTEXT-004](EV-W04-CONTEXT-004.png) | **11.7 us** |
| [CONTEXT-005](EV-W04-CONTEXT-005.png) | 11.0 us |
| [CONTEXT-006](EV-W04-CONTEXT-006.png) | 11.4 us |
| [CONTEXT-007](EV-W04-CONTEXT-007.png) | 11.33 us |

El máximo observado es **11.7 us**.

Cada 10 muestras de un muestreo de 1 kHz se libera control, es decir, hay
100 de estos hand-offs por segundo. La estimación solicitada es:

```text
100 hand-offs/s × 11.7 us = 1170 us/s = 1.17 ms/s = 0.117 % de 1 s
```

## Lectura y límite

El intervalo no es un microbenchmark puro de salvar/restaurar registros:
incluye `k_sem_give()`, la decisión de planificación y el inicio de
`control_thread`. Es el coste visible relevante para este diseño. El resultado
es un máximo de cinco pares inspeccionados de una captura de 3 s; no es una
cota formal de todos los cambios de contexto posibles.
