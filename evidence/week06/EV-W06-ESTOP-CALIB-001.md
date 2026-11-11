# EV-W06-ESTOP-CALIB-001 — E-stop y `calib` antes y después de las correcciones

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
- La diferencia de ciclos de `console_tid` entre dos comandos `threads` **no
  mide el CPU de `calib`**: el reporte de threads imprime cerca de 1 KB por UART
  a 115200 baudios en modo polling y su costo domina ese intervalo. Se descarta
  por tanto la cifra anterior de ≈115.6 ms como estimación de `calib`.
- Estimación indirecta según el método de la guía: en
  `EV-W05-PREEMPT-002.png`, entre el fin de una espera y el inicio de la
  siguiente transcurren aproximadamente 2.804 µs (330.254 → 333.058 µs).
  Multiplicado por las 1000 rondas de `calib`, da ≈**2.804 ms de CPU**. Es una
  estimación basada en la traza, no una lectura directa del contador de ciclos.

La duración de pared aumentó a unos 499 ms porque los 1000 `k_usleep(400)`
ceden CPU y se reanudan según los ticks y la planificación. Según
`EV-W05-CPU-001`, sampling, control, telemetry, display y flow ocupan en
conjunto aproximadamente 1.09 % del CPU observado; bajo esa carga queda cerca
de 989 ms de CPU por segundo. Así, tanto los ≈400 ms de CPU previos como la
estimación posterior de ≈2.8 ms caben en el presupuesto de un segundo. La
duración de pared posterior observada (499.4 ms) también cabe. Esto no es una
cota WCET: faltan los parámetros de las tareas de Daniela y una medición directa
de CPU de `calib` en la S3. Si el tick es de 1 ms, cada `k_usleep(400)` puede
redondearse a un tick, acercando la duración de pared a 1 s.

## Sumas de latencia del e-stop

- **Antes:** hasta una espera por el próximo control (10 ms) más un paso de PWM
  (1 ms), ≈**11 ms**; supera el requisito de 5 ms. El máximo observado en cinco
  trips fue 10.080 ms.
- **Después:** detección en sampling → latencia de liberación/dispatch de
  sampling + parte restante de `C_sampling` hasta escribir PWM bajo. El camino
  ya no espera al período de control y es del orden de microsegundos; se observó
  **0.2561 µs en un trip**. No se afirma máximo ni WCET: hacen falta `C_sampling`
  de la S3 y más trips post-arreglo.
