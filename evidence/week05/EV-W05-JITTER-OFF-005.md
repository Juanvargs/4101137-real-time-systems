# EV-W05-JITTER-OFF-005 — Jitter de muestreo con tracing desactivado

**Fecha y hora de la captura:** 2026-11-04 15:00:26.112 (marca de WaveForms)
**Requisito:** REQ-CTRL-03 (muestreo cada 1 ms)
**Placa:** ESP32-S3 DevKitC (identificador pendiente, OI-006)
**Build:** mismo firmware de Week 5 compilado sin `trace.conf` ni `uart_trace.overlay`
**Commit del firmware:** no registrado en la captura (pendiente de confirmar)
**Carga:** operación normal, sin comando de consola en curso (pendiente de confirmar)

## Instrumento y protocolo

- Analog Discovery 2, WaveForms Logic en modo *Record*, trigger *Normal*.
- DIO0 conectado a `instr_samp` (GPIO4 del S3).
- 30 000 000 muestras a 10 MHz: ventana de 3 s, resolución 0.1 µs,
  aproximadamente 3000 períodos de muestreo.
- Medición *Period* de WaveForms sobre DIO0 (estadística de toda la captura).

## Resultado

| Media | Mínimo | Máximo |
|---:|---:|---:|
| 1.00000 ms | 0.99770 ms | 1.0021 ms |

`J_obs,max` = max(1.0000 − 0.99770, 1.0021 − 1.0000) ms = **2.3 µs** (dominado por el período mínimo).

## Veredicto

CHARACTERIZED. Se compara con el registro par (tracing on/off) para estimar el
costo del observador: 12.7 − 2.3 = **10.4 µs** de aumento en el jitter máximo
observado. No es un límite garantizado.

## Archivos

- `EV-W05-JITTER-OFF-005.png`: captura de WaveForms con la medición.
- Limitación: no se exportaron los datos de transiciones ni el *workspace*;
  solo se conserva la captura de pantalla.
