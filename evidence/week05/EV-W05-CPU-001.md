# EV-W05-CPU-001 — Carga de CPU por hilo

**Fecha:** 2026-11-04
**Placa:** ESP32-S3 DevKitC
**Build:** firmware de Week 5 con trazado CTF activado
**Método:** comando `threads` (`thread_analyzer_print(0)`) en la consola USB-JTAG.

## Resultado

El analizador muestra `CPU: 0%` para la mayoría de los hilos porque presenta porcentajes enteros. Para conservar las participaciones menores a 1 %, se calcularon los porcentajes a partir de los ciclos acumulados que imprimió el analizador:

`porcentaje = ciclos del hilo / suma de ciclos de todos los hilos × 100`

| Hilo | Ciclos acumulados | CPU calculada |
|---|---:|---:|
| `idle` | 115301506313 | 97.99 % |
| `sampling_tid` | 1107319762 | 0.94 % |
| `tracing_thread` | 924815867 | 0.79 % |
| `console_tid` | 157654182 | 0.13 % |
| `control_tid` | 123958042 | 0.11 % |
| `telemetry_tid` | 37570546 | 0.03 % |
| `display_tid` | 15692922 | 0.01 % |
| `sysworkq` | 2960 | ≈ 0.00 % |
| `flow_wq` | 2145 | ≈ 0.00 % |
| `ISR0` | 0 | 0.00 % |
| **Total** | **117668522739** | **100.00 %** |

Los porcentajes son participaciones normalizadas de los ciclos acumulados desde el arranque; las cifras `CPU: n%` de la salida original son enteras y se redondean/truncan. `CONFIG_TRACING_IDLE=n` está configurado para reducir el volumen CTF, por lo que esta distribución se obtuvo con el analizador de hilos, que sí enumeró `idle` y `tracing_thread`.

## Transcripción recibida

```text
Thread analyze:
 flow_wq             : STACK: unused  720 usage  304 / 1024 ( 29 %); CPU:   0 %
                     : Total CPU cycles used: 2145
 telemetry_tid       : STACK: unused 1154 usage  894 / 2048 ( 43 %); CPU:   0 %
                     : Total CPU cycles used: 37570546
 sampling_tid        : STACK: unused  960 usage  576 / 1536 ( 37 %); CPU:   0 %
                     : Total CPU cycles used: 1107319762
 display_tid         : STACK: unused 1504 usage  544 / 2048 ( 26 %); CPU:   0 %
                     : Total CPU cycles used: 15692922
 control_tid         : STACK: unused  464 usage  560 / 1024 ( 54 %); CPU:   0 %
                     : Total CPU cycles used: 123958042
 console_tid         : STACK: unused  898 usage 1150 / 2048 ( 56 %); CPU:   0 %
                     : Total CPU cycles used: 157654182
 tracing_thread      : STACK: unused  448 usage  576 / 1024 ( 56 %); CPU:   0 %
                     : Total CPU cycles used: 924815867
 sysworkq            : STACK: unused  208 usage  304 /  512 ( 59 %); CPU:   0 %
                     : Total CPU cycles used: 2960
 idle                : STACK: unused   34 usage  222 /  256 ( 86 %); CPU:  97 %
                     : Total CPU cycles used: 115301506313
 ISR0                : STACK: unused    0 usage  512 /  512 (100 %); CPU:   0 %
                     : Total CPU cycles used: 0
```

**Formato de evidencia:** transcripción de la salida de consola compartida durante la sesión; no se adjuntó una captura de pantalla de esta salida.
