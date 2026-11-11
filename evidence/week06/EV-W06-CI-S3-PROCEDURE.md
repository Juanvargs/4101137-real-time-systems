# EV-W06-CI-S3 — Procedimiento para medir los anchos instrumentados

**Estado:** ambas variantes compilan; adquisición pendiente porque esta sesión
no detecta la ESP32-S3 ni el Digilent AD2 (`lsusb` solo mostró la webcam y el
adaptador inalámbrico). Build normal: `/tmp/w06-ci-s3`; build con OLED:
`/tmp/w06-ci-s3-display`.

Para flashear cuando esté conectada la S3, desde `~/zephyrproject`:

```bash
PATH="$HOME/.venv/bin:$PATH" west flash -d /tmp/w06-ci-s3 \
  --esp-device /dev/serial/by-id/usb-1a86_USB_Single_Serial_5C82127093-if00
```

Para medir display, flashea en cambio `/tmp/w06-ci-s3-display` y conecta el OLED.
Ambas variantes incluyen los mismos GPIO de instrumentación.

## Canales

Conecta tierra común entre la S3 y el AD2. Con el firmware
`firmware/kernel/boards/esp32s3_devkitc_esp32s3_procpu.overlay`, conecta:

| WaveForms | ESP32-S3 | Señal | Actividad a provocar |
|---|---:|---|---|
| DIO0 | GPIO4 | `instr_samp` | Muestreo periódico de 1 kHz |
| DIO1 | GPIO5 | `instr_ctrl` | Control periódico de 100 Hz |
| DIO2 | GPIO6 | `instr_cons` | Ejecutar `status` por consola; un pulso por comando/bytes según agrupación UART |
| DIO3 | GPIO7 | `instr_tele` | Salida de telemetría cada 1 s; el pulso incluye `printk` por UART polling |
| DIO4 | GPIO15 | `instr_flow` | Tarea de lotes; también es marcador de e-stop, mantener `estop=0` |
| DIO5 | GPIO17 | `instr_disp` | Refresco de pantalla; requiere SSD1306 conectado a I²C GPIO1/2 y build con `display.conf` |
| DIO6 (salida) | GPIO16 | Entrada de pulsos `flow_pulse` | Aplicar 100 pulsos digitales a 3.3 V para liberar un lote de flow |

No apliques 5 V a los GPIO. Para flow, configura DIO6 como salida digital de
3.3 V y conecta solo su señal y tierra común a GPIO16/GND.

## Captura

1. Arranca el firmware del kernel y confirma por UART que inicia sin errores.
   Limpia e-stop (`clear`) y deja la presión debajo del umbral.
2. En WaveForms abre Logic, selecciona DIO0–DIO5 como entradas, `Mode: Record`,
   `Samples: 30 M` y `Rate: 10 MHz`. Cada registro cubre aproximadamente 3 s.
   Añade una medida `PosWidth` para cada canal activo y guarda el registro.
3. Para sampling/control/telemetry, registra al menos tres ventanas. Guarda
   máximo, media y p99 de los anchos altos, y el número de pulsos observados.
   Un ancho durante ejecución concurrente es `C_obs` de pared: si una tarea de
   mayor prioridad la preempta, el pulso puede extenderse y no equivale por sí
   solo a CPU `C_i`.
4. Para consola, manda `status` desde Miniterm durante una captura. Evita usar
   `threads`: su reporte de ~1 KB por UART polling domina el costo y contamina
   la medida. La salida UART de `status` sí forma parte de ese camino medido.
   `calib` incluye `k_usleep`, así que su ancho GPIO es duración de pared y no
   CPU `C_i`.
5. Para flow, conéctalo a DIO6/GPIO16, emite 100 flancos ascendentes y mide DIO4.
   El pulso DIO4 marca la ejecución del lote, no el tiempo de cada ISR.
6. Para display, usa el OLED real y el build opcional `display.conf`; espera a
   que el primer refresco termine y mide DIO5 durante varias actualizaciones.
   Sin el dispositivo OLED listo no habrá pulso válido de ejecución.
7. Conserva los PNG originales y anota para cada señal la captura, configuración,
   ancho máximo/medio/p99, período/deadline usado y si hubo solapamiento con
   tareas de prioridad superior. Solo después rellena `c_ms` en
   `scripts/week06_rta_input.json`.

## Resultado pendiente

| Tarea | GPIO | `C_i`/ancho observado | Período | Deadline | Estado |
|---|---:|---:|---:|---:|---|
| sampling | 4 | — | 1 ms | 1 ms | Pendiente de captura S3 |
| control | 5 | — | 10 ms | 10 ms | Pendiente de captura S3 |
| telemetry | 7 | — | 1000 ms | 1000 ms | Pendiente de captura S3 |
| console (`status`) | 6 | — | Esporádica | Por definir | Pendiente de captura S3 |
| flow batch | 15 | — | 100 eventos | Por definir | Pendiente de estimular GPIO16 |
| display refresh | 17 | — | 500 ms | Por definir | Pendiente de OLED y captura S3 |

Esta tabla distingue parámetros del nodo de los valores de Task B aislado. No
se deben copiar `0.93830 ms` ni los bloqueos medidos en Task B como `C_i` del
firmware de SoilSense.
