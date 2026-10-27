# EV-W04-STACK-003 — High-water marks bajo carga

## Objetivo

Completar Task D: activar el analizador de hilos de Zephyr y medir el máximo
uso de pila después de actividad de flujo y de una calibración.

## Configuración y carga

- Placa/target: ESP32-C6 DevKitC compatible de 4 MB /
  `esp32c6_devkitc/esp32c6/hpcore`.
- Configuración añadida a `prj.conf`:

  ```ini
  CONFIG_THREAD_NAME=y
  CONFIG_THREAD_ANALYZER=y
  CONFIG_THREAD_ANALYZER_USE_PRINTK=y
  ```

- El comando UART `threads` llama a `thread_analyzer_print(0)`.
- Se conectó AD2 DIO4, como Pattern de 100 Hz, a GPIO3 (`flow_pulse`) y GND
  común. Tras más de 100 pulsos, `flow_wq` se ejecutó; la telemetría mostró
  `flow_x100=102`.
- Con el patrón activo se ejecutó `calib` y luego `threads`.
- La HMI SSD1306 no estaba conectada ni habilitada mediante `display.conf`;
  por tanto, el dato de `display_tid` corresponde al hilo y no a un redibujado
  I2C completo.

La salida original está preservada visualmente en
[EV-W04-STACK-004.png](EV-W04-STACK-004.png).

## Resultado de `threads`

| Hilo | Uso / asignado | Uso | Margen libre |
|---|---:|---:|---:|
| `flow_wq` | 268 / 1024 B | 26 % | 756 B |
| `telemetry_tid` | 660 / 2048 B | 32 % | 1388 B |
| `sampling_tid` | 252 / 1536 B | 16 % | 1284 B |
| `display_tid` | 220 / 2048 B | 10 % | 1828 B |
| `control_tid` | 236 / 1024 B | 23 % | 788 B |
| `console_tid` | 804 / 2048 B | 39 % | 1244 B |
| `sysworkq` | 272 / 512 B | 53 % | 240 B |
| `idle` | **228 / 256 B** | **89 %** | **28 B** |
| `ISR0` | 244 / 512 B | 47 % | 268 B |

## Veredicto

No se observó desbordamiento. `control_tid` usa 236 B de los 1024 B asignados,
por lo que la predicción conservadora de 512 B de uso máximo fue mayor que el
valor observado bajo esta carga. El hilo más cercano a su límite es `idle`,
con solo 28 B libres; entre los hilos de aplicación, el más cercano es
`console_tid` con 39 %. Antes de llevar esta configuración a un producto debe
revisarse el margen del hilo `idle` con la carga final y, probablemente,
aumentar su pila.
