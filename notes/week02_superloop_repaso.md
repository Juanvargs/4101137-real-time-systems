# Repaso — Week 2: superloop, ISR, jitter y bloqueo

## Qué construimos

Partimos de un firmware Zephyr con tareas ya implementadas, pero sin el mecanismo
que las llamaba. Completamos tres piezas:

1. `instr_set()` cambia los GPIO de instrumentación. Un pulso alto representa el
   tiempo durante el cual una tarea está ejecutándose.
2. `tick_isr()` incrementa `ticks_pending` cada 1 ms y actualiza
   `backlog_peak`. La ISR no lee el sensor.
3. `main()` implementa un superloop: consola → display → telemetría → un
   muestreo pendiente → control cada diez muestreos → lote de flujo.

No hay planificador ni prioridades entre tareas del loop. La tarea que se demora
retrasa a todas las que aparecen después de ella.

## Dos contextos de ejecución

```text
Interrupción de timer (cada 1 ms)      Superloop
---------------------------------      -----------------------------------
tick_isr()                             while (1) {
  ticks_pending++                        console, display, telemetry
  return                                 si hay tick: sampling
                                         cada 10: control
                                         flow batch
                                       }
```

Una ISR puede interrumpir temporalmente al superloop. Sin embargo, solo deja una
señal o contador atómico; el trabajo completo se realiza después en el loop.

## Qué significan las medidas

| Concepto | En este laboratorio |
|---|---|
| Período | Tiempo entre dos subidas consecutivas de DIO0. Nominal: 1 ms. |
| `C_i` observado | Ancho del pulso de instrumentación. Para sampling cache-on: 2–3 us. No es WCET probado. |
| Jitter máximo | Mayor desviación absoluta del período frente a 1 ms. |
| `backlog_peak` | Mayor número de ticks de 1 ms que esperaron a ser atendidos. |
| Latencia ISR→servicio | Desde la subida DIO6 (evento de flujo) hasta DIO4 (inicio de flow batch). |

La tasa de 1 MHz del AD2 equivale a una resolución de 1 us. Por eso los resultados
de microsegundos se redondean a esa resolución.

## Resultado baseline

El promedio de sampling fue 1.0001 ms, pero el período máximo fue 6.845 ms. El
jitter máximo fue 5.845 ms, de modo que el requisito hard de 1 ms **no se
cumple**, aunque el promedio se vea bueno.

Esto ocurre porque un período largo se compensa estadísticamente con varios
períodos muy cortos cuando el loop intenta atender los ticks acumulados. El
promedio no muestra el peor caso.

## Anchos observados de las demás tareas

También medimos cuánto permaneció alta la señal de instrumentación de cada tarea.
No son WCET: son los máximos vistos bajo el protocolo declarado.

| Tarea | Máximo observado | Qué significa |
|---|---:|---|
| Control | 0.7 us | Es corto; 300 ejecuciones en 3 s concuerdan con 10 ms. |
| Consola, `status` | 6.6010 ms | El Enter ejecuta e imprime la respuesta; los bytes simples duraron desde 4 us. |
| Telemetría | 5.7770 ms | Cada publicación de 1 Hz ocupa varios ms del único superloop. |
| Flow batch | 44 us | El cálculo se ejecuta una vez por cada 100 pulsos de flujo. |

El dato de telemetría es especialmente importante: una tarea soft relativamente
larga puede retrasar a una tarea hard si las dos comparten un superloop.

## Pulso de flujo

La AD2 generó DIO6 a 100 Hz y lo conectamos a D9. Después de 100 pulsos, el
firmware ejecuta `task_flow_batch()` y eleva DIO4. En un evento observado, la
subida de DIO4 ocurrió aproximadamente 6 us después de la subida DIO6 que causó
la interrupción. La bajada de DIO6 no importaba para esta medición: la ISR se
dispara en la subida.

## Experimento cache-off

La STM32G474 usa ART: caché de instrucciones, caché de datos y prefetch de Flash.
Adaptamos el fragmento del laboratorio —originalmente solo L4— para apagar estos
recursos también en G4. Con la caché apagada, el ancho promedio de sampling pasó
de 2.4886 us a 3.3888 us: la ejecución fue aproximadamente 36.2% más lenta.

El jitter máximo apenas cambió de 5.845 ms a 5.857 ms. Eso indica que el mayor
problema no era la velocidad de Flash, sino el bloqueo en el diseño del superloop.

## Experimento `calib`

`calib` ejecuta 1,000 veces `k_busy_wait(400)`. Es un bloqueo mínimo de 400 ms
dentro de `task_console()`. Las interrupciones de timer siguen contando, pero
`task_sampling()` no puede correr hasta que `cmd_calib()` termina.

El resultado medido fue:

```text
backlog_peak: 6 → 414 ticks
período máximo de sampling: 6.845 ms → 414.06 ms
jitter máximo: 5.845 ms → 413.06 ms
```

La coincidencia entre 414 ticks y aproximadamente 414 ms valida la explicación.
El requisito hard falló por más de 413 ms. Esta es la razón cuantitativa para
evaluar una arquitectura con planificador en las próximas semanas.

## Ideas que debes poder explicar

1. Una ISR puede contar un evento aunque el superloop esté ocupado.
2. Contar el evento no equivale a atenderlo a tiempo.
3. El promedio cercano a 1 ms no prueba cumplimiento de un plazo de 1 ms.
4. `C_obs,max` no es lo mismo que WCET: el primero solo es el mayor valor visto.
5. Apagar la caché aumentó el costo de ejecución, pero no arregla ni causa por sí
   solo el bloqueo de cientos de milisegundos.
6. Un comando firm puede ser funcionalmente correcto y, aun así, violar las
   garantías de una tarea hard si comparte el mismo superloop.
