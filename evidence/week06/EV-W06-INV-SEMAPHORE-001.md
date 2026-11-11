# EV-W06-INV-SEMAPHORE-001 — Inversión con semáforo

- **Firmware:** `firmware/week06_inversion/`, comando `s` (lock implementado como `k_sem`).
- **Prioridades:** control 2 (alta), medium 4, low 7 (baja).
- **Analizador:** DIO0=GPIO4 (low), DIO1=GPIO5 (control), DIO2=GPIO6 (medium); 30 M muestras a 10 MHz, 50 ms/div.

| Señal | Medición PosWidth |
|---|---:|
| DIO1 — respuesta de control | **50.466 ms** |
| DIO0 — low sostiene el lock | **51.604 ms** |
| DIO2 — trabajo de medium | **23.444 ms** |

La captura corregida libera control 2 ms después de que low toma el lock y
medium 3 ms después. DIO2 se ejecuta mientras DIO0 continúa sosteniendo el lock
y DIO1 está alto esperando; al acabar medium, low completa su trabajo y control
continúa. El baseline midió 0.93830 ms (`EV-W06-INV-BASELINE-001`).
