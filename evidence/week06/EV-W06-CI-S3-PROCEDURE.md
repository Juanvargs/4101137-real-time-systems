# EV-W06-CI-S3 — Procedimiento para medir los anchos instrumentados

**Estado:** adquisición parcial en ESP32-S3 + AD2. Se capturaron los anchos de
sampling, control, telemetry, console (`status`) y flow. La variante con display
produjo pulsos DIO5, pero el OLED no mostró imagen; ese ancho se conserva como
duración observada no validada y no se usa como `C_i`. Build normal:
`/tmp/w06-ci-s3`; build con OLED: `/tmp/w06-ci-s3-display`.

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
3. Para sampling/control/telemetry, registra al menos tres ventanas. En la
   tercera ventana se guardó el resumen WaveForms y los eventos de DIO0, DIO1
   y DIO3 en CSV. El p99 se calculó con rango más cercano sobre los anchos
   positivos completos. Los máximos de WaveForms se mantuvieron estables en las
   tres ventanas; las distribuciones CSV corresponden a una ventana de 3 s.
   Un ancho durante ejecución concurrente es `C_obs` de pared: si una tarea de
   mayor prioridad la preempta, el pulso puede extenderse y no equivale por sí
   solo a CPU `C_i`.
4. Para consola, manda `status` desde Miniterm durante una captura. Evita usar
   `threads`: su reporte de ~1 KB por UART polling domina el costo y contamina
   la medida. La salida UART de `status` sí forma parte de ese camino medido.
   `calib` incluye `k_usleep`, así que su ancho GPIO es duración de pared y no
   CPU `C_i`.
5. Para flow, se conectó DIO6/GPIO16 con reloj PP de 100 Hz durante 3 s. Se
   observaron 300 flancos de entrada y tres lotes en DIO4. El pulso DIO4 marca
   la ejecución del lote, no el tiempo de cada ISR.
6. Se probó el build opcional `display.conf` con OLED conectado. DIO5 mostró
   cinco pulsos de hasta 24.641 ms, pero como la pantalla no funcionó
   visualmente, se registra la observación como no validada y no se usa como
   `C_i`.
7. Los CSV de eventos y el resumen WaveForms quedan como evidencia primaria.
   Los anchos son `C_obs` bajo la carga capturada, no WCET; solo el subconjunto
   periódico medido se usa en un análisis RTA parcial.

## Resultados observados

| Tarea | GPIO | `C_i`/ancho observado | Período | Deadline | Estado |
|---|---:|---:|---:|---:|---|
| sampling | 4 | n=3000; media 0.93787 µs; p99 1.000 µs; máx. 4.200 µs | 1 ms | 1 ms | Observado en una ventana CSV; máx. estable en tres ventanas |
| control | 5 | n=300; media 351 ns; p99 400 ns; máx. 400 ns | 10 ms | 10 ms | Observado en una ventana CSV; máx. estable en tres ventanas |
| telemetry | 7 | n=3; media 63.967 µs; p99/máx. 64.000 µs | 1000 ms | 1000 ms | Solo tres ejecuciones; resultado preliminar |
| console (`status`) | 6 | n=8 pulsos; media 10.625 µs; p99/máx. 69.100 µs | Esporádica | Por definir | Mezcla recepción de bytes y manejador de fin de línea; no WCET |
| flow batch | 15 | n=3; media 2.467 µs; p99/máx. 2.500 µs | 1000 ms bajo estímulo 100 Hz | Por definir | Tres lotes inducidos con 300 flancos en 3 s |
| display refresh | 17 | n=5; media 24.630 ms; máx. 24.641 ms | 500 ms | Por definir | OLED sin imagen; duración observada no validada, excluida de `C_i` |

Esta tabla distingue parámetros del nodo de los valores de Task B aislado. No
se deben copiar `0.93830 ms` ni los bloqueos medidos en Task B como `C_i` del
firmware de SoilSense. Archivos: `EV-W06-CI-S3-periodic-003.csv` y los eventos
`EV-W06-CI-S3-DIO0-events.csv` hasta `EV-W06-CI-S3-DIO4-events.csv`. Los
resultados son observaciones de una ventana de 3 s por CSV; console y flow son
caminos activados manualmente. `display` y el conjunto completo permanecen
pendientes de validación.
