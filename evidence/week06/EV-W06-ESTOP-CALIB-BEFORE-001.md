# EV-W06-ESTOP-CALIB-BEFORE-001 — E-stop y `calib` antes de las correcciones

**Placa:** ESP32-S3 DevKitC · **Firmware:** `firmware/kernel`, estado previo a las correcciones de Task C · **Fecha:** 2026-11-10

## E-stop

Se desconectó la entrada de flujo y `instr_flow` marcó el instante en que
sampling detectó la condición. Se midió desde el flanco ascendente de DIO0
(GPIO15, detección) hasta el flanco descendente de DIO1 (GPIO21, válvula).

| Prueba | Demora detección → válvula baja |
|---:|---:|
| 1 | 1.024 ms |
| 2 | 2.010 ms |
| 3 | 10.080 ms |
| 4 | 9.987 ms |
| 5 | 1.012 ms |

Máximo observado: **10.080 ms**, por encima del límite de 5 ms.
La captura de ese máximo es `EV-W06-ESTOP-BEFORE-001.png`; la captura
post-arreglo es `EV-W06-ESTOP-AFTER-001.png`.

## Calibración

Con DIO3 conectado a GPIO6 (`instr_cons`), la anchura positiva durante el
comando `calib` fue **400.8 ms**. Antes de corregirlo, el manejador ejecutaba
1000 rondas de `k_busy_wait(400)`: **≈400 ms de CPU solicitada** por las
esperas, además del costo de las lecturas y del bucle. El pulso GPIO mide
duración de pared; la cifra de CPU es una estimación derivada del código, no
una medición independiente del contador de ciclos.
Capturas de ancho de pulso: `EV-W06-CALIB-BEFORE-001.png` y
`EV-W06-CALIB-AFTER-001.png`.

## Después de las correcciones

- E-stop: la captura post-arreglo mostró **256.1 ns** entre el flanco de
  detección y la bajada de la válvula, menor que el límite de 5 ms.
- `calib`: DIO3 mostró una duración de pared de **499.4 ms**.
- CPU de consola: `console_tid` pasó de 118,586,248 a 146,332,358 ciclos,
  una diferencia de **27,746,110 ciclos** (≈**115.6 ms** a 240 MHz). El
  intervalo incluye el comando `calib`, la entrada UART y parte del manejo de
  los dos comandos `threads`; por eso es una estimación ligeramente superior
  al CPU exclusivo de `calib`. Frente a los ≈400 ms solicitados por los
  `k_busy_wait(400)` originales, el uso observado cayó alrededor de 71 %.

La duración de pared aumentó a unos 499 ms porque los 1000 `k_usleep(400)`
ceden CPU y se reanudan según los ticks y la planificación. La duración de
pared y el tiempo de CPU son magnitudes diferentes.
