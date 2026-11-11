# EV-W06-EDF-001 — Fig. 4.13 con EDF

- **Placa:** ESP32-S3 DevKitC.
- **Configuración:** EDF habilitado; τ1 y τ2 con la misma prioridad estática.
- **Ejecución:** 60 s; `tau1_misses=0`, `tau2_misses=0`.
- **WaveForms:** DIO0 = GPIO4 (τ1), DIO1 = GPIO5 (τ2), 30 M muestras a 10 MHz, vista de 2 ms/div.

Lecturas visibles en la captura:

| Señal | Periodo | Ancho de pulso |
|---|---:|---:|
| DIO0 / τ1 | 4.9781 ms | 2.0011 ms |
| DIO1 / τ2 | 7.4671 ms | 4.0016 ms |

Los anchos de pulso corresponden aproximadamente a los tiempos de cómputo configurados (2 ms y 4 ms). La lectura de periodo de DIO1 es una medición puntual de esta vista; el resultado de 60 s confirmó cero deadlines perdidos.
