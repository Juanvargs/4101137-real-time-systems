# Guía de estudio y exposición de los laboratorios

Este documento reúne explicaciones rigurosas, pero fáciles de exponer, de los
laboratorios. Se irá completando en orden mientras repasamos cada semana. Al
final se añadirá una síntesis general de todo el trabajo.

Para cada semana se registrarán: objetivo, qué se hizo, qué evidencia se obtuvo,
qué significa el resultado, qué límites tiene y una explicación oral sugerida.

## Contenido

- [Semana 1 — Preparación del entorno](#semana-1--preparación-del-entorno-y-primera-ejecución)
- [Semana 2 — Mediciones del superbucle](#semana-2--mediciones-del-superbucle)
- [Semana 3 — Puerto al C6 e hilo de muestreo](#semana-3--puerto-al-c6-y-primer-hilo-de-muestreo)
- [Semana 4 — Migración completa a Zephyr](#semana-4--migración-completa-a-hilos-de-zephyr)
- [Semana 5 — CTF e instrumentación](#semana-5--trazado-ctf-y-costo-de-instrumentación)
- [Semana 6 — Planificación, temporización y seguridad](#semana-6--planificación-análisis-temporal-inversión-de-prioridad-y-seguridad)
- [Síntesis general](#síntesis-general-de-los-laboratorios)

---

## Semana 1 — Preparación del entorno y primera ejecución

### Objetivo

Dejar listo el entorno de Zephyr y verificar que se podía compilar, cargar y
ejecutar firmware tanto en una simulación como en la placa física. Esta semana
fue una puesta en marcha: todavía no se midieron tiempos de las tareas ni se
comprobó el cumplimiento de plazos del sistema SoilSense.

### Plataforma y herramientas

- La placa física disponible fue una **NUCLEO-G474RE**. STM32G474RE es el
  microcontrolador de la placa; NUCLEO-G474RE es el nombre de la placa completa
  y el objetivo de Zephyr es `nucleo_g474re`.
- Se verificó `west` versión 1.5.0 y un entorno Zephyr 4.4.99.
- Se instalaron/verificaron las herramientas de compilación cruzada para ARM y
  para ESP32-S3 (Xtensa), necesarias para compilar para esas arquitecturas.
- Se comprobó el acceso al puerto serial mediante el grupo `dialout`.
- Se usó pyOCD 0.45.1 con el depurador STLINK-V3 integrado en la placa.
- La evidencia registra GNU nano 7.2 como editor disponible; el comando de
  VS Code no se encontró. Esto no bloqueó el laboratorio.

### Qué se hizo y qué se observó

1. **Prueba sin placa:** se compiló `hello_world` para `native_sim` y se ejecutó
   en el computador. La compilación terminó correctamente y apareció el mensaje
   `Hello World! native_sim/native`.
2. **Compilación para la placa:** se compiló la aplicación Zephyr `blinky` para
   `nucleo_g474re`. El proceso terminó correctamente. El informe de compilación
   indicó 18 896 B de Flash usados (3,60 %) y 4 544 B de RAM (3,47 %).
3. **Prueba física:** se cargó `blinky` por STLINK-V3/pyOCD. Se observó el LED
   verde de usuario LD2 encendiéndose y apagándose aproximadamente una vez por
   segundo.
4. **Consola serial:** se modificó el texto de `hello_world`, se recompiló y se
   cargó. El monitor serial mostró el mensaje modificado a 115200 baudios, con
   configuración 8N1 (8 bits, sin paridad, 1 bit de parada).

### Qué demuestra y qué no

Las pruebas demuestran que el entorno podía compilar para `native_sim` y para
la NUCLEO-G474RE, que el firmware se podía cargar a la placa, que una salida
GPIO sencilla funcionaba y que se recibían mensajes por el puerto serial.

No demuestran que las tareas de SoilSense cumplan sus períodos o plazos: no se
midieron latencia, jitter ni tiempos de ejecución en la Semana 1. Tampoco se
deben confundir los bytes de Flash/RAM reportados por el compilador con la
cantidad escrita durante la programación; son medidas distintas.

La guía del curso menciona otras placas como referencia. El RET identifica la
NUCLEO-G474RE como la placa física usada y conserva la diferencia de plataforma
para no presentar sus resultados como si fueran de otra placa.

### Conceptos que debo poder explicar

- **West:** herramienta de comandos que organiza operaciones de Zephyr, como
  compilar y ejecutar aplicaciones.
- **Compilación cruzada:** generar un programa en el computador para que se
  ejecute en otro procesador, como el STM32 de la NUCLEO.
- **`native_sim`:** permite compilar y ejecutar una aplicación Zephyr como
  simulación en el computador; por sí solo no prueba el hardware físico.
- **GPIO:** entrada/salida digital; en esta prueba se observó mediante el LED.
- **Consola serial 115200 8N1:** canal de texto entre la placa y el computador;
  115200 es la velocidad y 8N1 describe el formato de cada carácter.

### Guion oral sugerido

> “En la Semana 1 preparé y comprobé el entorno Zephyr. Primero ejecuté
> `hello_world` en `native_sim`; luego compilé `blinky` para la placa
> NUCLEO-G474RE, lo cargué con STLINK-V3 y verifiqué que LD2 parpadeara.
> También cambié el mensaje de `hello_world` y confirmé que aparecía por la
> consola serial a 115200 baudios, 8N1. Esto verificó la cadena de compilación,
> programación, ejecución y comunicación. Todavía no medí tiempos ni plazos de
> SoilSense.”

### Evidencias del repositorio

- `evidence/week01/EV-W01-ENV-001.md`: herramientas y prueba `native_sim`.
- `evidence/week01/EV-W01-BUILD-002.md`: compilación de `blinky` y uso de memoria.
- `evidence/week01/EV-W01-HW-003.md`: programación y prueba física del LED.
- `evidence/week01/EV-W01-SERIAL-004.md`: mensaje modificado en la consola serial.

### Preguntas de comprobación

1. ¿Qué diferencia hay entre STM32G474RE y NUCLEO-G474RE?
2. ¿Qué verificó `native_sim` y qué verificó la prueba física de `blinky`?
3. ¿Por qué la Semana 1 no permite afirmar que el sistema cumple sus plazos?

---

## Semana 2 — Mediciones del superbucle

### Objetivo y configuración

En esta semana se midió el firmware de arquitectura **superbucle** en la placa
NUCLEO-G474RE. El temporizador generaba un tick cada 1 ms, pero el trabajo de
muestreo y las demás tareas se atendían desde el bucle principal. El Analog
Discovery 2 (AD2) y WaveForms observaron los GPIO de instrumentación.

La semana incluyó una línea base normal, una prueba con caché/acelerador Flash
desactivado, una prueba mientras se ejecutaba `calib`, y capturas de anchos de
pulsos de varias rutas. En la línea base no se ejecutaba un comando de consola;
en la prueba de bloqueo se envió `calib` por el puerto serial.

### Lectura paso a paso del diagrama `EV-W02-FLOW-001`

El dibujo separa tres caminos: el **bucle principal** en el centro, el **ISR del
temporizador** a la izquierda y el **ISR de flujo** a la derecha. ISR significa
rutina de interrupción: una función corta que se ejecuta al ocurrir un evento.
Las funciones llamadas `task_*` en este diagrama son pasos del superbucle, no
hilos separados del sistema operativo.

1. **Inicio e inicialización — `init_hw()` y `k_timer_start(..., 1 ms)`.** Se
   configuran los periféricos y se inicia el temporizador periódico.
2. **ISR del temporizador — `tick_isr()`.** Cada milisegundo aumenta
   `ticks_pending`. El ISR sí ejecuta código, pero solo el trabajo corto de
   registrar que llegó un tick; no lee el sensor ni ejecuta el muestreo.
   También actualiza `backlog_peak`, el mayor número de ticks que se han
   acumulado pendientes.
3. **Consola — `task_console()`.** Atiende caracteres y comandos que llegan por
   el puerto serial. `calib` se ejecuta desde esta ruta. Como en esta semana la
   consola forma parte del mismo superbucle, si un manejador se queda ocupado,
   retrasa los pasos que vienen después.
4. **Pantalla — `task_display()`.** Actualiza la interfaz visual, como los
   valores que muestra la pantalla. Está clasificada como trabajo *soft*:
   puede afectar la actualización visual sin ser la ruta rápida de seguridad.
5. **Telemetría — `task_telemetry()`.** Publica o transmite periódicamente el
   estado del sistema; el diagrama indica 1 Hz. También es trabajo *soft*.
6. **Decisión `ticks_pending > 0?`.** El bucle revisa si hay uno o más ticks de
   muestreo pendientes. Si no hay, omite el muestreo y continúa. Si sí hay,
   consume exactamente un tick y llama al bloque siguiente. Por eso una tarea
   bloqueante permite que el contador siga creciendo mientras el muestreo
   espera.
7. **Muestreo — `task_sampling()`.** Si había un tick pendiente, lee la entrada
   de presión y realiza un trabajo de muestreo. El objetivo es procesar una
   liberación de 1 kHz (cada 1 ms). Está marcado como trabajo *hard* por su
   importancia temporal.
8. **Decisión del control — `++control_div >= 10?`.** Después de cada
   `task_sampling()` ejecutado, suma uno al contador de muestras procesadas.
   Cuando llega a diez, es decir aproximadamente cada 10 ms, pone el contador
   en cero y llama a `task_control()`. El control calcula la acción
   necesaria y actualiza la salida correspondiente; también se clasifica como
   *hard*. Si aún no han pasado diez muestras, se omite en esa vuelta.
9. **ISR de flujo — `flow_isr()`.** En paralelo al bucle, un flanco de entrada
   del sensor llega por D9/PC7 y aumenta `flow_pulses`. El ISR sí ejecuta el
   incremento del contador; no hace el procesamiento más largo del lote.
10. **Lote de flujo — `task_flow_batch()`.** El bucle principal procesa el
    trabajo de flujo después de acumular 100 pulsos. Después vuelve al bloque
    3 y repite el ciclo.

La flecha azul muestra el orden normal del superbucle. Las flechas naranjas
punteadas muestran eventos de interrupción o datos que llegan de forma
asíncrona. La propiedad importante que ilustra el diagrama es que consola,
pantalla, telemetría, muestreo, control y lote de flujo se ejecutan por turnos
en el mismo bucle. No hay un planificador que permita a `task_sampling()`
interrumpir una función larga del bucle principal. El ISR de 1 ms puede seguir
contando ticks, pero el muestreo real espera hasta que el bucle vuelva a
atenderlo.

### Cómo leer período y fluctuación de muestreo

- El **período nominal** era 1 ms: se esperaba un nuevo muestreo cada milisegundo.
- El **período observado** es el tiempo real entre dos inicios consecutivos de
  muestreo.
- La **fluctuación máxima** (*maximum absolute jitter*) es la mayor diferencia
  absoluta entre un período observado y el período esperado de 1 ms.
- En la línea base, el período máximo fue 6,845 ms; por tanto, la fluctuación
  máxima fue 6,845 − 1 = **5,845 ms**. El período ya excedía el límite de 1 ms
  antes de ejecutar `calib`.
- Durante `calib`, el período máximo fue 414,06 ms; la fluctuación máxima fue
  414,06 − 1 = **413,06 ms**. No significa que cada muestreo se retrasara esa
  cantidad: es el peor intervalo observado de la captura.

El período medio de 1,0001 ms no contradice esos máximos. Un promedio puede
ocultar intervalos muy largos y otros muy cortos; por eso también se revisan el
mínimo, el máximo y la fluctuación.

### Qué significa `backlog_peak`

El ISR del temporizador de 1 kHz incrementa un contador de ticks pendientes; no
hace el muestreo. El bucle principal atiende los ticks después, y procesa como
máximo un trabajo de muestreo pendiente por vuelta. `backlog_peak` es el máximo
número de ticks de 1 ms que quedaron pendientes a la vez.

Durante `calib`, la espera ocupada bloqueó el bucle principal mientras el ISR
seguía contando los ticks. El máximo registrado llegó a **414 ticks**. Esos
ticks representan cerca de 414 ms de liberaciones pendientes, coherente con el
período máximo de 414,06 ms que midió el AD2. El backlog no es una fluctuación:
es un contador de trabajo temporal pendiente; el jitter expresa cuánto se
desvió el intervalo de muestreo de 1 ms.

### Resultado de caché y bloqueo

| Medición | Caché activada, línea base | Caché desactivada | Durante `calib` |
|---|---:|---:|---:|
| Período medio de muestreo | 1,0001 ms | 1,0001 ms | 1,0001 ms |
| Período máximo | 6,845 ms | 6,857 ms | 414,06 ms |
| Fluctuación máxima | 5,845 ms | 5,857 ms | 413,06 ms |
| Ancho medio del pulso de muestreo | 2,4886 μs | 3,3888 μs | 2,5071 μs |
| `backlog_peak` | 6 | No consultado por separado | 414 |

Desactivar la caché aumentó el ancho medio observado del trabajo de muestreo en
36,2 %, pero la fluctuación máxima solo cambió de 5,845 a 5,857 ms: 12 μs. La
interferencia más grande vino del bloqueo síncrono del superbucle, no de la
caché. Con `calib`, el período máximo y el backlog crecieron enormemente porque
el bucle principal no podía atender los trabajos mientras permanecía ocupado.

### Anchos de otras rutas observadas

| Ruta | Resultado máximo observado | Cómo interpretarlo |
|---|---:|---|
| Control | 0,7 μs | 300 ejecuciones observadas en 3 s; concuerda con activación nominal cada 10 ms. |
| Consola, un comando `status` | 6,6010 ms | La captura mezcla pulsos cortos de recepción de caracteres con el manejador de fin de línea de `status`; no es un tiempo genérico de consola. |
| Telemetría | 5,7770 ms | Máximo observado durante la captura de 30 s. |
| Lote de flujo | 44 μs | Máximo observado con estímulo de entrada de 100 Hz. |

Estos máximos son observaciones (`C_obs`), no límites garantizados de peor caso
(WCET). Para un pulso de GPIO, el analizador mide cuánto tiempo estuvo activo el
marcador; no demuestra por sí solo que todos los futuros trabajos duren menos.

### Mapa de evidencias para revisarlas una por una

1. `EV-W02-FLOW-001`: dibujo del superbucle y conexión entre interrupciones,
   tareas y GPIO de instrumentación.
2. `EV-W02-BASELINE-002`: muestreo en la línea base, sin comando de consola.
3. `EV-W02-FLOWLAT-003`: latencia observada entre pulso de flujo y servicio.
4. `EV-W02-CACHE-004`: comparación con caché activada y desactivada.
5. `EV-W02-BLOCK-005`: efecto del comando bloqueante `calib`.
6. `EV-W02-TASKS-006`: anchos observados de control, consola, telemetría y flujo.

### Qué está midiendo cada captura de resultados

**Aclaración de nombres:** `DIO0`, `DIO1`, etc. son canales del analizador AD2.
No son los pines `D0`, `D1` de la NUCLEO. La instrumentación relacionó los
canales AD2 con marcadores del firmware: DIO0 muestreo, DIO1 control, DIO2
consola, DIO3 telemetría, DIO4 lote de flujo y DIO6 estímulo de flujo.

1. **Línea base — `EV-W02-BASELINE-002-statistics.png` y `-overview.png`.**
   “Línea base” significa funcionamiento normal para comparar: caché/ART
   activada, sin comando de consola y generador de patrones detenido. Se mide
   el período entre pulsos de muestreo DIO0 y el ancho de cada pulso. El AD2
   registró 30 s a 1 MHz. Resultado: promedio 1,0001 ms, máximo 6,845 ms y ancho
   medio 2,4886 μs. La imagen de estadísticas muestra números; la vista general
   permite ver la captura completa.
2. **Caché apagada — `EV-W02-CACHE-004-statistics.png`.** Se repitió el mismo
   firmware y carga normal de la línea base; la diferencia intencional fue
   desactivar las cachés ART de instrucciones/datos y la precarga Flash del
   STM32. Se mide el mismo período DIO0 y ancho del pulso de muestreo. Resultado:
   período medio 1,0001 ms, máximo 6,857 ms y ancho medio 3,3888 μs. La ejecución
   del muestreo observada fue más larga en promedio; el jitter máximo cambió
   solo 12 μs respecto a línea base, por lo que la caché no explica el bloqueo
   dominante.
3. **Durante `calib` — `EV-W02-BLOCK-005-statistics.png`.** Se volvió a usar el
   firmware con caché activada y se envió `calib` por serial mientras el AD2
   medía DIO0. Se mide el período de muestreo y el ancho del marcador. Resultado:
   período medio 1,0001 ms, máximo 414,06 ms, fluctuación máxima 413,06 ms,
   ancho medio 2,5071 μs y ancho máximo 12 μs. La captura muestra el período
   extremo; el valor de `backlog_peak=414` viene del firmware y está escrito en
   el registro EV-W02-BLOCK-005, no se lee en la tabla de WaveForms.
4. **Control — `EV-W02-TASKS-006-control.png`.** Captura normal de 3 s a 10 MHz.
   Se mide el ancho positivo del marcador DIO1 de control: 300 pulsos, media
   0,633 μs, mínimo 0,600 μs y máximo 0,700 μs. También aparece DIO0, que marca
   muestreo, pero el resultado de control se lee en la fila DIO1.
5. **Consola — `EV-W02-TASKS-006-console.png`.** Un comando `status` durante una
   captura de 30 s a 1 MHz. DIO2 marca pulsos relacionados con recepción y
   manejo del comando; se contaron ocho bytes/pulsos: media 0,85037 ms, mínimo
   4 μs y máximo 6,6010 ms. Los pulsos incluyen recepción corta de caracteres y
   el manejador de fin de línea; por eso la media no es una duración genérica de
   una operación de consola.
6. **Telemetría — `EV-W02-TASKS-006-telemetry.png`.** Funcionamiento normal de
   30 s a 1 MHz. Se mide el ancho positivo DIO3: 30 pulsos, media 5,7682 ms,
   mínimo 5,6900 ms y máximo 5,7770 ms.
7. **Lote de flujo — `EV-W02-TASKS-006-flow.png`.** Se aplicó estímulo de flujo
   a 100 Hz durante 30 s y se midió el marcador DIO4. Hubo 30 lotes; media
   34,033 μs, mínimo 33 μs y máximo 44 μs.
8. **Latencia de flujo — `EV-W02-FLOWLAT-003-latency.png`.** En esta prueba el
   AD2 generó pulsos de entrada a 100 Hz y conectó su canal **DIO6** al pin D9/PC7
   de la NUCLEO. DIO6 representa el estímulo que imita los pulsos del sensor de
   flujo; no es una tarea ni un pin llamado D6 en la placa. El firmware cuenta
   esos flancos en `flow_isr()` y, al completar 100 pulsos, el superbucle inicia
   `task_flow_batch()`. Un GPIO del firmware marca ese procesamiento y se
   observa en el canal **DIO4** del AD2 (conectado a D7/PA8 de la NUCLEO).

   La medición usa dos cursores para encontrar la diferencia de tiempo entre el
   **flanco ascendente de DIO6** —llega el pulso 100 del estímulo— y el **flanco
   ascendente de DIO4** —empieza el lote de procesamiento—. WaveForms muestra
   `ΔX = 6.046 μs`: en ese evento, el servicio comenzó unos 6 μs después de la
   entrada. No mide cuánto dura todo el lote; eso se mide con el ancho del pulso
   DIO4 en la captura de anchos de tareas. Es una latencia observada para un
   evento seleccionado, no una garantía ni un plazo formal del flujo.

Los anchos de pulso indican cuánto permaneció alto un marcador colocado alrededor
del código instrumentado. Son duraciones observadas en esas capturas y cargas; no
son límites garantizados de peor caso (WCET).

### Guion oral sugerido

> “En la Semana 2 empezamos a medir el superbucle en la NUCLEO-G474RE. El
> muestreo debía ocurrir cada 1 ms. Aunque el promedio fue 1,0001 ms, la línea
> base tuvo un período máximo de 6,845 ms, es decir, 5,845 ms de fluctuación
> máxima respecto al objetivo. Al ejecutar `calib`, el bucle principal quedó
> bloqueado: el período máximo subió a 414,06 ms y `backlog_peak` llegó a 414
> ticks pendientes. También medimos pulsos de control, consola, telemetría y
> flujo; son tiempos observados, no WCET. La evidencia muestra que el problema
> dominante era el trabajo bloqueante dentro del superbucle.”

### Evidencias del repositorio

- Índice: `evidence/week02/README.md`.
- Arquitectura: `EV-W02-FLOW-001.md`, `EV-W02-FLOW-001.png` y `EV-W02-FLOW-001.svg`.
- Línea base: `EV-W02-BASELINE-002.md`, `EV-W02-BASELINE-002-overview.png` y `EV-W02-BASELINE-002-statistics.png`.
- Flujo: `EV-W02-FLOWLAT-003.md`, `EV-W02-FLOWLAT-003-overview.png` y `EV-W02-FLOWLAT-003-latency.png`.
- Caché desactivada: `EV-W02-CACHE-004.md` y `EV-W02-CACHE-004-statistics.png`.
- Bloqueo por `calib`: `EV-W02-BLOCK-005.md` y `EV-W02-BLOCK-005-statistics.png`.
- Anchos por ruta: `EV-W02-TASKS-006.md` y las capturas `EV-W02-TASKS-006-control.png`, `-console.png`, `-telemetry.png` y `-flow.png`.

### Preguntas de comprobación

1. ¿Por qué el promedio de 1,0001 ms no demuestra que todos los muestreos
   ocurrieron dentro de 1 ms?
2. ¿Cuál es la diferencia entre un backlog de 414 ticks y 413,06 ms de
   fluctuación máxima?
3. ¿Por qué no debemos atribuir el gran retraso principalmente a la caché?

---

## Semana 3 — Puerto al C6 y primer hilo de muestreo

### Qué cambió

Se llevó el firmware al ESP32-C6 DevKitC compatible y se movió **solo el
muestreo** desde el superbucle a un hilo de Zephyr. Se compararon tres casos:
NUCLEO-G474RE con superbucle (Semana 2), ESP32-C6 con superbucle y ESP32-C6 con
un hilo de muestreo. La comparación entre las dos pruebas del C6 es la más
directa; la NUCLEO y el C6 son placas distintas, así que sus diferencias no se
deben atribuir únicamente a la arquitectura.

El ISR periódico siguió siendo corto: registra cada liberación de 1 ms y la
coloca en `tick_q`; no hace por sí mismo la conversión del sensor. El nuevo
`sampling_thread`, de prioridad 2, toma esos ticks y realiza el muestreo. El
bucle principal (`main`) tiene prioridad 10; en Zephyr un número menor indica
una prioridad mayor. Por eso el hilo de muestreo puede ejecutarse antes que el
trabajo de prioridad baja de `main`.

### Comparación de resultados

| Medición | NUCLEO-G474RE con superbucle | C6 con superbucle | C6 con hilo de muestreo |
|---|---:|---:|---:|
| Fluctuación máxima de muestreo en reposo | 5,845 ms | 105 μs | 2 μs |
| Fluctuación máxima con `calib` | 413,06 ms | 402,47 ms | 2 μs |
| `backlog_peak` con `calib` | 414 ticks | 403 ticks | 0 ticks |
| Tick hasta inicio del hilo de muestreo | — | — | 11 μs máximo observado |
| Período máximo de control con `calib` | No medido | 412,47 ms | 412,31 ms |

### Cómo interpretar la diferencia entre muestreo y control

**El período de muestreo sí mejoró con el hilo.** En el C6 con superbucle,
durante `calib` se observó un período máximo de muestreo de 403,47 ms, equivalente
a una fluctuación máxima de 402,47 ms respecto al objetivo de 1 ms. Con el hilo,
el período máximo observado fue 1,0020 ms: solo 2 μs por encima del objetivo.
Además, el backlog bajó de 403 ticks pendientes a cero.

**El período de control casi no mejoró.** Pasó de 412,47 ms a 412,31 ms porque
el control todavía se ejecutaba desde el bucle principal; no se había movido a
un hilo propio. `calib` seguía corriendo sincrónicamente en `main` y podía
bloquearlo. El hilo nuevo protegió el muestreo, pero `task_control()` tenía que
esperar a que `main` regresara de `calib`. Por eso los valores cercanos a 412 ms
son del control, no una señal de que el muestreo haya quedado igual.

La latencia de **11 μs** mide desde el tick/liberación hasta que comienza el
hilo de muestreo. No es el ancho de ejecución del muestreo: la captura de ancho
separada observó un máximo `C_obs` de 2,7 μs. Una medida es cuánto esperó para
empezar; la otra, cuánto duró el tramo instrumentado.

### Resultado y límite de la Semana 3

El hilo de Zephyr protegió el muestreo del trabajo largo de consola: durante
`calib`, la fluctuación observada bajó de cientos de milisegundos a 2 μs y no se
acumularon ticks pendientes. Sin embargo, el período máximo de muestreo de
1,0020 ms todavía excedió el intervalo objetivo de 1 ms por 2 μs, y no se midió
por separado un límite de terminación de cada trabajo. El control siguió
bloqueado en `main`. Es una mitigación específica del muestreo, no todavía una
migración completa del sistema a hilos.

El puerto C6 usó una superposición de Devicetree para reflejar la memoria Flash
física de 4 MB. Las mediciones del C6 no son mediciones del ESP32-S3; las placas
tienen procesadores y subsistemas de memoria/serie distintos.

### Variables compartidas que aparecen al crear el hilo

- `pressure_mv`: presión medida; la escribe el muestreo y la leen control,
  telemetría, pantalla y consola.
- `estop`: indica la parada de emergencia; puede actualizarse desde muestreo,
  joystick y algunas rutas de consola; varias tareas la consultan.
- `setpoint_mv`: presión objetivo; la actualizan consola o joystick y la leen
  control y otras rutas.

En esta etapa el acceso se aceptó provisionalmente para el experimento, pero no
era una prueba completa de sincronización. La protección de datos compartidos,
mutexes e inversión de prioridades se estudió después.

### Guion oral sugerido

> “En la Semana 3 comparamos el superbucle de la NUCLEO, el superbucle del C6 y
> el C6 con un hilo de muestreo. En el C6 con superbucle, `calib` elevó la
> fluctuación de muestreo a 402,47 ms y el backlog a 403 ticks. Al poner el
> muestreo en un hilo de mayor prioridad, la fluctuación bajó a 2 μs y el
> backlog a cero. Pero el control siguió con un período máximo cercano a 412 ms
> porque todavía vivía en `main`, que `calib` bloqueaba. Por eso protegimos el
> muestreo, pero aún no el control.”

### Evidencias del repositorio

Consultar `RET.md` §3.4 y los registros individuales en `evidence/week03/`
(esa carpeta no tiene un README índice). La secuencia de capturas que estamos
revisando es:

1. `EV-W03-PORT-001.png`: salida de consola que confirma la prueba del port; no
   es una medición temporal con WaveForms.
2. `EV-W03-BASELINE-002.png`: línea base de muestreo del C6 con superbucle.
3. `EV-W03-SUPERLOOP-CONTROL-006.png` y
   `EV-W03-SUPERLOOP-CALIB-007.png`: control y muestreo/control durante
   `calib`, todavía en superbucle.
4. `EV-W03-THREAD-CALIB-004.png` y `EV-W03-THREAD-IDLE-005.png`: versión con
   el hilo de muestreo, durante `calib` y en reposo.
5. `EV-W03-CI-008.png`: ancho del pulso de ejecución de `task_sampling()`.

#### Imagen 1: el port funcionando — `EV-W03-PORT-001.png`

Es una captura de la terminal, no del analizador. Muestra `miniterm` abierto a
115200 baudios en `/dev/ttyACM0` (consola USB Serial/JTAG de la ESP32-C6) con
el superbucle portado. Cada línea `t=... p_mv=... sp_mv=... duty=...` es la
telemetría de 1 Hz: `t` avanza 1000 ms entre líneas, la presión ronda el
setpoint de 1500 mV y el `duty` del PI está entre 43 y 46 %. En medio se envió
`status`, que responde con el mismo estado y `backlog_peak=51`.

Lo que demuestra es que el mismo código C compiló, se cargó y corre en otra
placa solo cambiando el overlay de Devicetree: el control funciona y la consola
responde. **No es una medición temporal**: no hay tiempos de GPIO. El
`backlog=51` indica que en ese superbucle llegaron a acumularse 51 ticks
pendientes, una señal de que el bucle ya se retrasaba, pero el retraso se mide
en las capturas siguientes con el AD2.

#### Imagen 2: línea base C6 — `EV-W03-BASELINE-002.png`

Esta es una captura de 30 s del ESP32-C6 DevKitC compatible con el superbucle,
sin `calib`. El AD2 tomó 30 millones de muestras a 1 MHz. DIO0 está conectado
al marcador GPIO18 de `task_sampling()` y por eso la fila DIO0 Period mide el
tiempo entre pulsos de muestreo. WaveForms registró 29 999 períodos: media
1,00000 ms, mínimo 0,89700 ms y máximo 1,1050 ms. Frente al objetivo de 1 ms,
la mayor diferencia absoluta es 105 μs; el período máximo excedió 1 ms por esa
cantidad. Esta observación no es un WCET ni mide por separado cuánto tarda en
completarse cada trabajo.

DIO3 marca telemetría: se observaron 29 períodos de aproximadamente 1 s, lo
que concuerda con su cadencia de 1 Hz. Aunque la pantalla también muestra una
fila de período de DIO1, el registro no la usa para el período de control:
1 MHz no resolvió de forma fiable sus pulsos cortos; ese control se volvió a
capturar a 5 MHz en otra evidencia.

#### Imagen 3: control del superbucle durante `calib` — `EV-W03-SUPERLOOP-CONTROL-006.png`

Esta repetición se capturó en el C6 a 5 MHz durante 6 s, mientras se ejecutaba
`calib`. DIO1 marca control. WaveForms encontró 599 intervalos: media 10,000 ms,
mínimo 58,600 μs y máximo 406,48 ms. La media está cerca del período esperado
de control (10 ms), pero el máximo muestra que hubo una pausa enorme. El mínimo
muy corto también muestra que algunas ejecuciones quedaron muy juntas al
retomarse el superbucle. Esta repetición registró 406,48 ms; el RET conserva
412,47 ms como máximo entre las capturas válidas de control durante `calib`.
Esta imagen apoya el bloqueo del control, pero no reemplaza la captura
simultánea principal de `EV-W03-SUPERLOOP-CALIB-007.png`.

#### Imagen 4: muestreo y control del superbucle durante `calib` — `EV-W03-SUPERLOOP-CALIB-007.png`

Esta es la captura simultánea principal del C6 con superbucle durante el comando
`calib`: 30 millones de muestras a 5 MHz, una ventana de 6 s. WaveForms mide
períodos de DIO0 (muestreo) y DIO1 (control); DIO2 marca la actividad de consola.
En esta captura, los canales del analizador AD2 están cableados así: DIO0 del
AD2 observa GPIO18 (`instr_samp`) del C6; DIO1 observa GPIO19 (`instr_ctrl`);
DIO2 observa GPIO20 (`instr_cons`). Son pines de instrumentación que el
firmware sube y baja para que podamos ver cuándo ejecuta cada ruta: no son los
pines reales del sensor de presión, la válvula ni el UART.
Durante el pulso de consola se ve una pausa larga en las señales de muestreo y
control, y luego pulsos acumulados al reanudarse el bucle.

- DIO0, muestreo: objetivo 1 ms; media 1,0000 ms; máximo 403,47 ms; mínimo
  5,6 μs. El máximo excede el período objetivo en 402,47 ms, así que
  `J_obs,max=402,47 ms`.
- DIO1, control: objetivo 10 ms; media 10,000 ms; máximo 412,47 ms; mínimo
  58,6 μs. El control también sufre una pausa extrema porque sigue en el mismo
  bucle principal que `calib`.
- DIO2 marca cuándo está activa la ruta de consola que ejecuta `calib`.

Los promedios cercanos a 1 ms y 10 ms no eliminan los incumplimientos: la captura
contiene muchos períodos normales y también huecos muy largos. Los mínimos muy
cortos son compatibles con pulsos que quedan agrupados cuando el superbucle
retoma trabajo pendiente. La consola reportó por separado `backlog_peak=403`,
coherente con los ticks de muestreo acumulados durante el bloqueo. Esta captura
demuestra que en el superbucle `calib` retrasa a la vez muestreo y control; no
mide la duración de ejecución interna de cada uno de esos trabajos.

#### Imagen 5: muestreo con hilo durante `calib` — `EV-W03-THREAD-CALIB-004.png`

Esta captura de 6 s a 5 MHz se tomó en el mismo C6 mientras corría `calib`, pero
con `task_sampling()` ya atendido por `sampling_thread`. DIO0 marca muestreo,
DIO1 control, DIO2 consola y DIO3 telemetría.

- DIO0: período objetivo 1 ms; media 1,00000 ms; máximo 1,0020 ms. La fluctuación
  máxima observada es 2 μs. Se ven muestreos regulares durante el pulso de
  consola, en lugar del gran hueco de la captura de superbucle.
- DIO1: período objetivo 10 ms; media 10 ms; máximo 412,31 ms. El control
  todavía sufre una pausa grande porque permanece dentro de `main`.
- DIO2 identifica visualmente el período en que la consola atiende `calib`.
  DIO3 marca la telemetría.

Después de la captura, el firmware reportó `backlog_peak=0` y
`lat_peak_us=11`. Esos dos números vienen del reporte del firmware, no de la
tabla de períodos de WaveForms. La evidencia muestra la mitigación selectiva:
el hilo de mayor prioridad protege el muestreo del bloqueo de `main`, pero no
protege todavía el control. El máximo del muestreo aún excede 1 ms por 2 μs y
no es una prueba formal de cumplimiento.

#### Imagen 6: hilo de muestreo en reposo — `EV-W03-THREAD-IDLE-005.png`

Esta captura corresponde a la versión con `sampling_thread`, sin comando
`calib`. Es una de cinco capturas consecutivas: 30 millones de muestras a
5 MHz guardan 6 s cada una, así que las cinco juntas cubren 30 s. La imagen
preservada es la quinta captura.

- DIO0, muestreo: 5999 intervalos; media 1,00000 ms; mínimo 0,99800 ms; máximo
  1,0020 ms. El máximo se desvía 2 μs del objetivo de 1 ms.
- DIO1, control: 599 intervalos; media 10,000 ms; mínimo 9,9978 ms; máximo
  10,001 ms en esta captura.
- DIO3 muestra la telemetría a aproximadamente 1 Hz.

Esta captura confirma que, en reposo y durante este tramo, las señales se
mantienen cerca de sus períodos nominales. El resultado global de 30 s se basa
en las cinco capturas: los máximos de muestreo fueron 1,0018; 1,0016; 1,0020;
1,0020 y 1,0020 ms. La fluctuación global observada fue 2 μs. Sigue siendo un
resultado observado, no una garantía WCET, y el máximo de 1,0020 ms aún supera
el objetivo estricto de 1 ms por 2 μs.

### Preguntas de comprobación

1. ¿Qué valor demuestra que el hilo protegió el muestreo?
2. ¿Por qué el período máximo de control siguió cerca de 412 ms?
3. ¿Qué diferencia hay entre la latencia tick-a-hilo de 11 μs y el ancho de
   ejecución observado de 2,7 μs?

---

## Semana 4 — Migración completa a hilos de Zephyr

### Qué cambió respecto a la Semana 3

En la Semana 3 solo el muestreo tenía un hilo; control, consola, pantalla,
telemetría y flujo seguían dependiendo del bucle principal. En la Semana 4 se
completó la migración: `main` inicializa el sistema, arranca el temporizador y
termina; las tareas periódicas o de servicio quedan en hilos del kernel.

| Tarea o servicio | Mecanismo | Prioridad | Función resumida |
|---|---|---:|---|
| `sampling_tid` | Hilo, consume `tick_q` | 2 | Atiende cada liberación de muestreo de 1 ms. |
| `control_tid` | Hilo, espera `control_sem` | 3 | Se despierta después de cada décimo muestreo, aproximadamente cada 10 ms. |
| Flujo | `flow_wq`, trabajo diferido | 5 | La interrupción registra los pulsos; la cola de trabajo procesa el lote. |
| `display_tid` | Hilo dormido/periódico | 7 | Actualiza la pantalla cuando está habilitada. |
| `telemetry_tid` | Hilo dormido/periódico | 8 | Publica telemetría, aproximadamente a 1 Hz. |
| `console_tid` | Hilo | 9 | Atiende comandos seriales, incluido `calib`. |

En este rango preventivo de Zephyr, **un número de prioridad menor significa
mayor prioridad**. Por eso sampling (2) puede interrumpir a console (9) y
control (3) tiene prioridad mayor que flow/display/telemetry/console.

### Cómo se despierta el control

El hilo de muestreo procesa ticks de 1 ms. Cada vez que completa diez muestras,
da el semáforo `control_sem`. El hilo `control_tid` estaba esperando ese
semáforo; al recibirlo queda listo para correr. Muestreo tiene prioridad 2 y
control prioridad 3, así que el control corre cuando el muestreo cede o termina.
El semáforo es la señal de “ya hay trabajo de control”; no ejecuta el control
por sí mismo.

### Conceptos: semáforo y pila

- **Semáforo (`control_sem`):** es una señal con contador que coordina dos
  partes del sistema. Aquí `sampling_tid` lo entrega cada décima muestra y
  `control_tid` espera esa señal. Al recibirla, el control queda listo para
  ejecutarse. El semáforo sirve para avisar que hay trabajo; no es el área de
  memoria del hilo y no reemplaza un mutex para proteger datos compartidos.
- **Pila de un hilo:** es la zona de memoria temporal que cada hilo usa mientras
  ejecuta funciones: guarda, por ejemplo, variables locales y el punto al que
  debe regresar cuando termina una función. Cada hilo tiene una cantidad
  asignada. Si la ejecución necesita más espacio del asignado, puede desbordar
  la pila y corromper memoria o hacer fallar el programa.
- **High-water mark de pila:** es la mayor cantidad de pila que llegó a usarse
  durante la prueba. Restarla de la pila asignada da el margen libre observado.
  El analizador de Week 4 mide ese uso bajo una carga específica; no garantiza
  que otra carga futura nunca use más.

### Comparación A/B en el C6

Se comparó el superbucle de la Semana 3 con el kernel completo de la Semana 4
en la misma ESP32-C6, con los mismos marcadores y carga. Es una comparación
más controlada que comparar la NUCLEO con el C6.

| Medición | C6 con superbucle | C6 con kernel completo | Lectura |
|---|---:|---:|---|
| Fluctuación máxima de muestreo en reposo | 105 μs | 6 μs | Mejora observada de 99 μs. |
| Fluctuación máxima de muestreo con `calib` | 402,47 ms | 6,2 μs | El hilo de muestreo evita el bloqueo de cientos de ms. |
| Período máximo de control con `calib` | 412,47 ms | 10,003 ms | El control también queda separado de la consola, pero aún supera 10 ms. |
| Tick hasta inicio del muestreo (`lat_peak_us`) | — | 10 μs | Nueva medida de demora de liberación a inicio. |

Durante `calib`, el período máximo de muestreo fue **1,0056 ms**: excedió el
objetivo de 1 ms por 5,6 μs. El período máximo de control fue **10,003 ms**:
excedió 10 ms por 3 μs. El firmware reportó `backlog_peak=0`, señal de que la
cola de liberaciones no se acumuló durante esa prueba.

El resultado es una mejora grande, pero no un PASS temporal estricto. Estos
datos miden intervalos entre pulsos/liberaciones observadas; no establecen por
separado un límite de terminación para cada trabajo y no son WCET.

### Costo visible del traspaso muestreo → control

Con cursores del AD2 se midió desde la bajada del marcador de muestreo DIO0
hasta la subida del marcador de control DIO1. Cinco lecturas fueron 11,1; 11,7;
11,0; 11,4 y 11,33 μs. El máximo observado fue **11,7 μs**.

Como el control se libera cada diez muestreos de 1 kHz, hay unos 100 traspasos
por segundo. La estimación es `100 × 11,7 μs = 1,17 ms/s`, equivalente a
**0,117 % de un segundo**. Incluye la señal del semáforo, la decisión del
planificador y el inicio del hilo de control; no es solo el tiempo de guardar y
restaurar registros del procesador.

### Uso de pila

El analizador de Zephyr midió el uso máximo de pila bajo pulsos de flujo de
100 Hz, ejecución de `calib` y consulta `threads`. No observó desbordamientos.

- `idle`: 228 de 256 B usados (89 %), solo **28 B libres**; es el margen más
  estrecho.
- `control_tid`: 236 de 1024 B usados (23 %), 788 B libres.
- `console_tid`: 804 de 2048 B usados (39 %), el mayor uso entre los hilos de
  aplicación de esa captura.

La pantalla SSD1306 opcional no estaba conectada/habilitada, así que este
resultado no incluye una actualización completa por I2C. El margen pequeño de
`idle` requiere revisión bajo la carga final.

### Conclusión de la Semana 4

La migración completa a hilos elimina el gran retraso del superbucle y mantiene
la fluctuación de muestreo en unos pocos microsegundos incluso con `calib`.
También separa el control de la consola, pero las mediciones máximas todavía
superan ligeramente los períodos objetivo: 1,0056 ms frente a 1 ms y 10,003 ms
frente a 10 ms. La conclusión rigurosa es **mitigación importante demostrada;
cumplimiento hard real-time aún no demostrado**.

### Guion oral sugerido

> “En la Semana 4 pasamos también control, consola, telemetría, pantalla y flujo
> a mecanismos del kernel. El hilo de muestreo recibe los ticks; cada diez
> muestras libera el semáforo que despierta al control. En la comparación sobre
> el mismo C6, con `calib` la fluctuación de muestreo bajó de 402,47 ms a
> 6,2 μs y el período máximo de control bajó de 412,47 ms a 10,003 ms. La mejora
> fue grande, pero el muestreo todavía llegó a 1,0056 ms y el control a
> 10,003 ms; por eso no afirmamos que los plazos estrictos quedaron probados.”

### Evidencias del repositorio

- `evidence/week04/EV-W04-AB-001.md`: comparación de períodos y capturas
  `EV-W04-KERNEL-IDLE-001.png` y `EV-W04-KERNEL-CALIB-002.png`.
- `evidence/week04/EV-W04-COST-002.md`: cinco capturas de traspaso
  `EV-W04-CONTEXT-003.png` a `EV-W04-CONTEXT-007.png`.
- `evidence/week04/EV-W04-STACK-003.md` y `EV-W04-STACK-004.png`: uso de pila.

Para revisar las imágenes de Week 4 en orden:

1. `EV-W04-KERNEL-IDLE-001.png`: períodos de muestreo/control con el kernel en
   reposo.
2. `EV-W04-KERNEL-CALIB-002.png`: períodos con `calib` activo.
3. a 7. `EV-W04-CONTEXT-003.png` a `EV-W04-CONTEXT-007.png`: cursores para
   medir el traspaso del hilo de muestreo al de control.
8. `EV-W04-STACK-004.png`: salida del analizador de pilas bajo carga.

#### Imagen 1: `EV-W04-KERNEL-IDLE-001.png` — kernel en reposo

La captura registra 30 s a 1 MHz. DIO0 es el marcador de ejecución de
`sampling_tid`; la tabla mide su período, es decir, el tiempo entre pulsos
consecutivos. Se ven 29 999 períodos, con media de 1,0000 ms, mínimo de
0,9940 ms y máximo de 1,0040 ms. Frente al objetivo de 1 ms, las desviaciones
extremas son 6 μs por debajo y 4 μs por encima; por tanto, la fluctuación
absoluta máxima observada es 6 μs.

DIO1 es el marcador de control, pero esta captura toma una muestra cada 1 μs.
Como los pulsos de control son mucho más cortos, el analizador puede perder
flancos. Por eso la tabla muestra solo 2 468 períodos y valores de hasta
40,003 ms; **no deben presentarse como el período real del control**. Para
comparar el control se usa la captura con `calib`, tomada a 5 MHz, que sí
resuelve sus pulsos y da un máximo de 10,003 ms. Esta captura de reposo sirve
para el resultado de muestreo, no para concluir que el control tuvo un período
medio de 12,152 ms.

#### Imagen 2: `EV-W04-KERNEL-CALIB-002.png` — `calib` durante la captura

Aquí el AD2 registra 6 s a 5 MHz mientras se ejecuta `calib` por la consola.
La resolución es de 0,2 μs por muestra, así que esta captura sí permite
resolver el marcador rápido de control. DIO0 (muestreo) tiene 5 999 períodos:
promedio 1,0000 ms, mínimo 0,99380 ms y máximo 1,0056 ms. Respecto al objetivo
de 1 ms, el peor desvío absoluto es 6,2 μs. DIO1 (control) tiene 599 períodos,
promedio 10,000 ms, mínimo 9,9966 ms y máximo 10,003 ms; el máximo supera los
10 ms nominales por 3 μs.

La columna `Period` mide el tiempo entre pulsos consecutivos de ese marcador;
no es el ancho de ejecución de la tarea. La captura muestra que el `calib` ya
no bloquea muestreo y control durante cientos de milisegundos, como ocurría en
el superbucle. Aun así, los dos máximos observados exceden ligeramente sus
períodos objetivo: son una mejora grande, pero no demuestran cumplimiento
estricto ni un WCET. El `control_sem` es parte de cómo se libera el control
cada diez muestreos; el pulso DIO1 permite observar el ritmo resultante, no
ver directamente la operación interna del semáforo.

#### Imagen 3: `EV-W04-CONTEXT-003.png` — un traspaso muestreo → control

Esta es una captura ampliada a 10 MHz (resolución de 0,1 μs). El cursor X1
marca el flanco de bajada de DIO0, cuando termina el marcador del trabajo de
muestreo; X2 marca el flanco de subida siguiente de DIO1, cuando empieza el
marcador del trabajo de control. WaveForms informa `ΔX = 11,1 μs` en esta
muestra. La cifra `1/ΔX = 90,09 kHz` que también aparece en el cuadro es solo
el inverso matemático del intervalo; no es la frecuencia del muestreo ni del
control.

El intervalo refleja el traspaso que importa a esta arquitectura: terminar el
marcador de muestreo, entregar `control_sem`, planificar y empezar el control.
No aísla solamente el costo interno de cambiar registros del procesador. Esta
es una de cinco muestras; el máximo observado en la serie fue 11,7 μs.

#### Imagen 4: `EV-W04-CONTEXT-004.png` — mayor traspaso observado

Se repite el mismo método y la misma conexión de la imagen anterior, pero en
otro par de pulsos. El cursor X1 marca la bajada de DIO0 y X2 la subida de
DIO1; aquí `ΔX = 11,7 μs`. Es el mayor de los cinco intervalos registrados.
Se interpreta como el tiempo visible entre el fin del marcador de muestreo y
el comienzo del marcador de control, incluyendo la notificación del
semáforo y la planificación. No es el ancho del pulso de control ni una cota
WCET.

#### Imagen 5: `EV-W04-CONTEXT-005.png` — otra muestra del traspaso

WaveForms muestra `ΔX = 11,0 μs` para este par de cursores. Es el menor de los
cinco intervalos recogidos, no el menor tiempo de ejecución de `sampling_tid`
ni de `control_tid`. Las capturas 003–007 se combinan para resumir el mismo
intervalo de fin de muestreo a comienzo de control.

En el conjunto de cinco muestras, el mínimo observado es 11,0 μs, el máximo
11,7 μs y el promedio aritmético 11,306 μs. Estos estadísticos describen el
traspaso entre las dos tareas, no sus anchos de ejecución por separado.

#### Imagen 6: `EV-W04-CONTEXT-006.png` — otra muestra del traspaso

En esta captura WaveForms muestra `ΔX = 11,4 μs`. Se suma a las otras cuatro
observaciones del intervalo entre el fin del marcador de muestreo y el inicio
del marcador de control. Está entre el mínimo de 11,0 μs y el máximo de
11,7 μs; no cambia esos extremos ni el propósito de la medición.

#### Imagen 7: `EV-W04-CONTEXT-007.png` — quinta muestra del traspaso

Mismo método: X1 en la bajada de DIO0 (fin del muestreo) y X2 en la subida
siguiente de DIO1 (inicio del control), con la vista a 5 μs/div. WaveForms
muestra `ΔX = 11,33 μs`, la quinta y última muestra de la serie. Con ella se
completan los cinco valores (11,1; 11,7; 11,0; 11,4 y 11,33 μs) que dan el
mínimo de 11,0 μs, el máximo de 11,7 μs y el promedio de 11,306 μs. La tabla
de Measurements muestra `Count 0` porque en esta vista no se pidió ninguna
estadística: el dato sale de los cursores.

#### Imagen 8: `EV-W04-STACK-004.png` — uso de pila bajo carga

Es la salida de terminal del comando `threads` (`thread_analyzer_print`) en el
C6, tomada después de aplicar pulsos de flujo de 100 Hz con el AD2 y de
ejecutar `calib`. Para cada hilo muestra la pila usada sobre la asignada y el
porcentaje. El dato clave es `idle`: 228 de 256 B (89 %), con solo **28 B
libres**, el margen más estrecho. Entre los hilos de la aplicación, el más
cargado es `console_tid` con 804 de 2048 B (39 %). `control_tid` usa
236 de 1024 B (23 %). No hubo desbordamiento, pero es una medición bajo esta
carga concreta; el display no estaba conectado, así que no incluye un redibujado
completo por I2C.

### Preguntas de comprobación

1. ¿Qué hace `control_sem` y por qué no ejecuta el control por sí mismo?
2. ¿Qué mejoró con el kernel completo y qué valores aún excedieron los
   objetivos?
3. ¿Qué incluye la medida de 11,7 μs y por qué no es un microbenchmark puro de
   cambio de contexto?

---

## Semana 5 — Trazado CTF y costo de instrumentación

### Objetivo

Observar internamente el orden en que Zephyr atiende interrupciones y cambia
entre hilos, medir cuánto CPU consume cada hilo y comprobar cuánto altera el
trazado el período de muestreo. La placa de esta semana fue la ESP32-S3 DevKitC;
por eso sus valores no son una comparación A/B directa con los del C6 de Week 4.

### Conceptos

- **Trazado (tracing):** registrar eventos mientras el programa corre, por
  ejemplo entrada a una interrupción, un hilo que queda listo, un hilo que
  deja la CPU o uno que empieza a ejecutarse. Sirve para reconstruir el orden y
  los tiempos internos del sistema.
- **CTF:** Common Trace Format, formato estructurado en que se guardan los
  eventos del trazado, con marcas de tiempo y datos como el nombre del hilo.
  En esta prueba los eventos se enviaban por UART al visor de trazas.
- **Instrumentación:** código añadido para medir o registrar el sistema. El
  trazado añade eventos y trabajo de transporte; por eso puede consumir CPU y
  cambiar un poco los tiempos que intenta observar.
- **Preempción:** cuando un hilo de prioridad mayor interrumpe a uno de menor
  prioridad para usar la CPU. Aquí el tick hace que `sampling_tid` quede listo;
  al ser más prioritario que `console_tid`, el muestreo lo interrumpe.
- **Cambio de contexto:** el kernel guarda el estado del hilo que deja la CPU y
  restaura el del que empieza. El intervalo de 4,837 μs entre dos eventos CTF
  de cambio de hilo es una transición concreta consola → muestreo; no es el
  mismo intervalo que el traspaso GPIO muestreo → control de Week 4.
- **Latencia ISR → inicio del hilo:** tiempo desde la entrada al ISR del tick
  hasta que el marcador indica que empezó `sampling_tid`. En una observación
  CTF fue 20,012 μs; `lat_peak_us=30 μs` era el máximo acumulado del firmware,
  así que no tienen que corresponder al mismo tick.
- **Porcentaje de CPU por hilo:** fracción de los ciclos acumulados atribuida
  a cada hilo. Es una proporción de esa ejecución desde el arranque, no una
  duración de cada trabajo ni una garantía de carga futura.
- **Búfer de trazado:** memoria finita donde se acumulan eventos antes de
  enviarlos. Si se llena, los eventos nuevos pueden perderse; una traza
  incompleta no significa que la tarea haya terminado.
- **Espera ocupada y espera que cede CPU:** `k_busy_wait(400)` mantiene el hilo
  activo durante la espera y no permite que un hilo de menor prioridad corra.
  Una espera suspendida, como `k_usleep`, deja la CPU disponible mientras
  espera y puede tardar más en tiempo de pared por el tick y el planificador.

### Qué se hizo

1. Se activó el trazado CTF en el firmware de la ESP32-S3 y se siguieron los
   eventos durante operación normal y durante `calib`.
2. Se examinó una interrupción de tick y la preempción de la consola por
   `sampling_tid`; también se siguió la señal del semáforo que despierta
   `control_tid` cada décimo muestreo.
3. Se consultó `threads` y se calcularon porcentajes con los ciclos acumulados,
   porque el comando imprimía porcentajes enteros y mostraba muchos hilos como
   0 %.
4. Se comparó el jitter de muestreo con CTF encendido y apagado, usando AD2 en
   ambas capturas: 30 M muestras a 10 MHz durante 3 s.

### Qué muestran los resultados

**La traza explica el orden.** En un tick, el ISR entró a 130,925 μs,
`sampling_tid` empezó a ejecutarse a 150,937 μs y luego se bloqueó esperando el
siguiente tick; entonces `console_tid` reanudó `calib`. La diferencia ISR →
inicio de muestreo de ese evento fue 20,012 μs. El intervalo de 4,837 μs mide
otra cosa: el espacio entre los hooks CTF de salida de consola y entrada de
muestreo.

En el décimo tick, `sampling_tid` entregó el semáforo a 12,171 ms y
`control_tid` quedó listo a 12,175 ms. El muestreo siguió corriendo hasta que
se bloqueó; después `control_tid` empezó a 12,195 ms y salió de la espera del
semáforo a 12,199 ms. Así se ve que `k_sem_give()` despierta/prepara el control,
pero no le quita instantáneamente la CPU a un hilo más prioritario.

**El trazado tuvo un costo medible.** El AD2 observó estos períodos de DIO0,
marcador de muestreo:

| CTF | Período medio | Mínimo | Máximo | Jitter absoluto máximo |
|---|---:|---:|---:|---:|
| Activado | 0,99999 ms | 0,98730 ms | 1,0065 ms | 12,7 μs |
| Desactivado | 1,00000 ms | 0,99770 ms | 1,0021 ms | 2,3 μs |

La diferencia entre los jitters máximos observados es **10,4 μs** (12,7 −
2,3). La predicción era 10 μs. Es el costo observado del trazado en esta
ejecución de la S3; no es una constante ni un límite garantizado. El AD2 mide
los bordes del GPIO desde fuera; CTF sirve para explicar quién corrió y en qué
orden. Sus números no tienen por qué coincidir porque observan eventos
distintos y CTF añade trabajo al firmware.

**El analizador estimó el reparto de CPU.** A partir de los ciclos reportados,
`idle` ocupó 97,99 %, `sampling_tid` 0,94 %, `tracing_thread` 0,79 %,
`console_tid` 0,13 %, `control_tid` 0,11 %, telemetría 0,03 % y display 0,01 %;
los demás fueron prácticamente cero. Se normalizaron los ciclos por la suma
total porque la salida entera ocultaba valores menores a 1 %. Es una foto
acumulada desde el arranque bajo esa ejecución. `CONFIG_TRACING_IDLE=n`
reduce la cantidad de eventos de inactividad en la traza; el analizador de
hilos sí reportó `idle`.

**La traza de `calib` se quedó corta.** El búfer CTF era de 32 KiB. Durante
`calib`, `console_tid` repetía `k_busy_wait(400)` sin ceder la CPU; el hilo que
vacía/transmite la traza no alcanzaba a hacerlo con suficiente rapidez. Solo
quedaron 189 pares de eventos: 189 × 400 μs = 75,6 ms. El comando completo
duraba alrededor de 400 ms; el resto no aparece porque se saturó el búfer.
La falta de eventos no implica que `calib` se detuviera a los 75,6 ms.

### Límites y conclusión

Los tiempos CTF son observaciones instrumentadas: el trazador consume tiempo y
el búfer perdió parte de la ejecución de `calib`. Además, el análisis de CPU
es acumulado y el A/B de jitter es de la ESP32-S3, mientras que el traspaso
GPIO de Week 4 (11,7 μs) se midió en ESP32-C6. No deben mezclarse como si fueran
una prueba en la misma placa o el mismo punto de medición. Para el período y
jitter se cita AD2; para el orden de ejecución y la causa de las preempciones,
CTF.

### Guion oral sugerido

> “En la Semana 5 activamos CTF para ver por dentro el orden de interrupciones
> y tareas, medimos el reparto de CPU y comparamos el muestreo con el trazado
> activado y desactivado. La traza confirmó que el muestreo de prioridad alta
> interrumpe la consola y mostró cómo el semáforo libera el control. El AD2
> midió que el jitter máximo observado aumentó de 2,3 a 12,7 μs, unos 10,4 μs
> atribuibles al trazado en esa ejecución. El registro de `calib` quedó
> incompleto porque el búfer CTF se llenó mientras la consola hacía esperas
> ocupadas. Por eso usamos CTF para entender la secuencia y AD2 para medir los
> períodos.”

### Evidencias

- `evidence/week05/EV-W05-PREEMPT-002.png` y su versión anotada: secuencia de
  preempción durante `calib`; el `.md` contiene marcas de tiempo.
- `EV-W05-CONTROL-003.png` y versión anotada: entrega y recepción del semáforo.
- `EV-W05-CPU-001.md`: ciclos de hilos y porcentajes normalizados.
- `EV-W05-JITTER-ON-004.png` y `EV-W05-JITTER-OFF-005.png`, con sus `.md`:
  comparación AD2 con CTF activado/desactivado.
- Limitaciones pendientes en los registros: confirmar identificación de la
  placa y commit exacto del firmware; no se conservaron exportaciones de datos
  de transición ni workspace de WaveForms.

#### Imagen 1: `EV-W05-PREEMPT-002.png` — la traza durante `calib`

Cada fila es un evento y la columna `time` es su marca de tiempo desde el
origen de la traza. `event` dice qué ocurrió; `fields` da detalles, como el
nombre del hilo. Los valores hexadecimales de `id` identifican objetos/hilos
internos de Zephyr; para explicar la secuencia basta con los nombres legibles.

La parte central comienza a 130,925 μs con `isr_enter` (entra el ISR del tick).
A 138,342 μs, `sampling_tid` queda `ready`: ya puede correr, pero todavía no
está usando la CPU. A 146,100 μs, `console_tid` sale (`thread_switched_out`), y
a 150,937 μs `sampling_tid` entra (`thread_switched_in`). La diferencia entre
estos dos hooks de cambio es 4,837 μs; desde entrada al ISR hasta inicio de
muestreo son 20,012 μs. Son dos intervalos distintos.

Las llamadas `gpio_port_set_bits_raw_*` y `gpio_port_clear_bits_raw_*`
corresponden al manejo de pines de instrumentación. `pins=16` es una máscara
de bits que señala GPIO4; no significa canal DIO16 del AD2. Las filas `enter`
y `exit` permiten ver que una llamada instrumentada a poner el pin tomó
aproximadamente 2,633 μs en este evento; esa ruta también lleva costo de
trazado.

A 174,533 μs, `sampling_tid` hace `thread_sched_pend`: se bloquea esperando el
siguiente tick. Luego sale de la CPU y `console_tid` reanuda a 185,679 μs, en
su `k_busy_wait(400)`. `thread_busy_wait_enter/exit` marca esas esperas de
calibración. La secuencia demuestra preempción por prioridad, pero la captura
no contiene toda la calibración: el búfer CTF se llena.

#### Imagen 2: `EV-W05-CONTROL-003.png` — el semáforo libera el control

La zona resaltada registra el décimo tick. A 12,171 ms ocurre
`semaphore_give_enter`: `sampling_tid` entrega el semáforo. A 12,175 ms
`control_tid` queda `ready`; a 12,178 ms termina la operación `give`. El
muestreo sigue ejecutándose y se bloquea a 12,184 ms; sale de la CPU a
12,189 ms. Recién a 12,195 ms Zephyr cambia a `control_tid`, y a 12,199 ms
termina la espera de semáforo (`semaphore_take_exit`).

La secuencia ilustra tres estados distintos: el semáforo se entrega, el
control queda listo y, después, recibe la CPU. `k_sem_give()` no ejecuta el
control ni lo pone a correr mientras sigue ejecutándose el hilo de muestreo
de mayor prioridad. Las operaciones GPIO de las filas siguientes marcan el
inicio del código instrumentado de control.

#### Imagen 3: `EV-W05-JITTER-ON-004.png` — muestreo con CTF activado

El canal DIO0 está conectado al marcador `instr_samp` del ESP32-S3. La captura
tiene 30 millones de muestras a 10 MHz: 3 s de señal con resolución de 0,1 μs.
La tabla `Period` resume el intervalo entre pulsos sucesivos de muestreo; no
mide el ancho del pulso ni el tiempo de ejecución de `task_sampling()`.

El promedio fue 0,99999 ms, el mínimo 0,98730 ms y el máximo 1,0065 ms. Frente
al objetivo de 1 ms, el mínimo quedó 0,01270 ms = 12,7 μs por debajo; el
máximo quedó 6,5 μs por encima. Por eso el jitter absoluto máximo es 12,7 μs.
La captura es la condición con CTF encendido; la comparación válida viene de
la siguiente captura, con el mismo AD2 y el trazado apagado.

#### Imagen 4: `EV-W05-JITTER-OFF-005.png` — muestreo con CTF apagado

Se conserva el mismo canal DIO0 y el mismo protocolo del AD2: 30 millones de
muestras a 10 MHz durante 3 s. La diferencia experimental es que el firmware
se compiló sin la configuración de tracing. WaveForms da promedio 1,00000 ms,
mínimo 0,99770 ms y máximo 1,0021 ms. Frente a 1 ms, los desvíos son 2,3 μs
por debajo y 2,1 μs por encima; el jitter absoluto máximo observado es
2,3 μs.

Al compararlo con la imagen 3 (CTF encendido), la diferencia entre los jitter
máximos es 12,7 − 2,3 = 10,4 μs. Esto estima el aumento observado en esas
ejecuciones, no un costo fijo de CTF por evento. La ficha de evidencia deja
pendiente confirmar el identificador físico de la placa, el commit exacto y
la carga idéntica entre capturas; por eso el RET lo presenta como costo
observado/caracterizado, no como prueba causal definitiva ni límite.

#### Imágenes 5–6: versiones anotadas de las trazas

`EV-W05-PREEMPT-002-annotated.png` y
`EV-W05-CONTROL-003-annotated.png` repiten las capturas anteriores, con
recuadros y flechas que destacan la preempción y la entrega del semáforo. No
son mediciones nuevas ni cambian los timestamps: ayudan a ubicar visualmente
los eventos ya explicados. En la captura anotada del control, `pins=32` es la
máscara del GPIO5 usado para marcar la entrada a la ruta de control.

### Preguntas de comprobación

1. ¿Por qué el intervalo CTF de 4,837 μs y el traspaso GPIO de 11,7 μs de la
   Semana 4 no miden lo mismo?
2. ¿Qué instrumento usamos para dar el número del jitter y cuál para explicar
   el orden de ejecución? ¿Por qué?
3. ¿Por qué la traza de `calib` solo conserva unos 75 ms si el comando dura
   unos 400 ms?

## Semana 6 — Planificación, análisis temporal, inversión de prioridad y seguridad

### Propósito

La Semana 6 une el comportamiento del planificador con el análisis de plazos y
con dos problemas concretos del sistema: el acceso a un recurso compartido y
la respuesta de emergencia. También mide cuánto duran varias rutas de
instrumentación del firmware SoilSense en la ESP32-S3. Hay que distinguir los
modelos didácticos (Fig. 4.13 e inversión de prioridad) de la medición del
nodo SoilSense: no son un único conjunto de tareas ni una única prueba.

### Vocabulario y conceptos

- **Trabajo/tarea periódica:** se libera repetidamente cada período `T`. En la
  Fig. 4.13, `τ1` tiene `T=5 ms` y `τ2`, `T=7 ms`.
- **Tiempo de ejecución `C_i`:** tiempo de CPU que un trabajo necesita para
  ejecutar su código, sin contar la espera causada por otras tareas. En un
  análisis formal se necesita una cota WCET; un máximo observado en una
  captura solo es `C_obs,max`, no WCET.
- **Deadline `D_i`:** instante límite para terminar el trabajo. El período es
  cuándo llega el siguiente trabajo; el deadline es cuándo debe terminar el
  actual. En Fig. 4.13 se usa `D=T`.
- **Respuesta `R_i`:** tiempo de pared desde que se libera el trabajo hasta
  que termina. Puede ser mayor que `C_i` por preempción y bloqueo.
- **Utilización `U`:** fracción ideal de CPU que requiere un conjunto:
  `U = Σ(C_i/T_i)`. Si `U>1`, la carga no cabe en un solo procesador. Si
  `U≤1`, eso por sí solo no prueba que RM sea planificable. Para EDF, con
  tareas periódicas independientes y deadlines implícitos (`D=T`), `U≤1` es
  condición **necesaria y suficiente**: si se cumple, EDF es planificable, y si
  no se cumple, ningún algoritmo lo es.
- **RM (Rate Monotonic):** prioridad fija; cuanto menor es el período, mayor
  prioridad. En el ejemplo `τ1` (5 ms) tiene prioridad sobre `τ2` (7 ms).
- **EDF (Earliest Deadline First):** prioridad dinámica; corre primero el
  trabajo listo cuyo deadline absoluto está más cerca.
- **RTA (Response-Time Analysis):** cálculo iterativo de la respuesta más
  desfavorable, incluyendo la interferencia de tareas más prioritarias.
  Compara `R_i` con `D_i`.
- **Hiperperíodo:** mínimo común múltiplo de los períodos. Para 5 y 7 ms es
  35 ms; después se repite la pauta de liberaciones ideal.
- **Inversión de prioridad:** una tarea alta queda bloqueada por un recurso
  que posee una tarea baja, mientras una tarea media preempta a la baja y
  prolonga indirectamente la espera de la alta.
- **Semáforo y mutex:** ambos coordinan acceso/espera, pero el semáforo usado
  como lock no aplica herencia de prioridad. Un mutex Zephyr sí puede elevar
  temporalmente la prioridad del dueño mientras una tarea de mayor prioridad
  espera. La herencia reduce la inversión; no elimina la duración de la sección
  crítica ni garantiza que el deadline se cumpla.
- **Bloqueo `B`:** tiempo durante el que un trabajo no puede progresar porque
  espera un recurso. En las capturas de inversión, el ancho del GPIO de low
  mientras sostiene el lock es un `B` observado para ese ensayo, no una cota
  universal.
- **E-stop:** parada de emergencia. Aquí se mide desde la detección de la
  condición hasta que la señal de la válvula baja. El requisito citado es
  5 ms.
- **Tiempo de pared frente a CPU:** tiempo de pared incluye espera, preempción
  y planificación; tiempo de CPU es el tiempo en que el procesador ejecuta esa
  ruta. `k_busy_wait()` consume CPU mientras espera; `k_usleep()` cede la CPU.

### A. Fig. 4.13: RM frente a EDF

El conjunto didáctico es `τ1: C=2 ms, T=D=5 ms` y
`τ2: C=4 ms, T=D=7 ms`. Su utilización es:

```text
U = 2/5 + 4/7 = 0.971429 ≈ 97.14 %
```

El cálculo dice que queda alrededor de 2,86 % de CPU libre en el modelo ideal.
No significa “cada tarea usa el 97 %”; significa que entre las dos consumen
ese porcentaje de un procesador ideal.

**Con RM**, `τ1` siempre tiene prioridad mayor por tener el período más corto.
En el primer ciclo: `τ1` usa CPU de 0 a 2 ms; `τ2` corre de 2 a 5 ms, pero
solo completa 3 de sus 4 ms de CPU. En `t=5 ms` se libera `τ1` otra vez y la
preempta; `τ2` conserva 1 ms pendiente. Luego termina cerca de `t=8 ms`, aunque
su deadline era `t=7 ms`.

RTA lo expresa con:

```text
R2 = 4 + ceil(R2/5) · 2
iteraciones: 4 → 6 → 8 → 8 ms
```

Converge a `R2=8 ms`, mayor que `D2=7 ms`; predice que `τ2` puede fallar. El
ensayo de 60 s observó `tau1_misses=0`, `tau2_misses=3429`. WaveForms midió un
ancho máximo del pulso DIO1 de `τ2` de 6,0099 ms. `τ2` necesita 4 ms de CPU,
pero el pulso queda alto durante unos 6 ms de pared porque `τ1` le quita cerca
de 2 ms. El log de misses confirma las fallas; el ancho del pulso ilustra la
preempción, pero por sí solo no mide cuánto tarde respecto a su deadline.

El modelo ideal predice aproximadamente una falla de `τ2` por hiperperíodo de
35 ms (unas 1714 en 60 s); se observaron 3429. El costo de GPIO, el bucle y el
planificador pueden explicar de forma plausible fallas adicionales de pocos
microsegundos, pero no se aisló esa causa. Presentarla como hipótesis coherente,
no como causa probada.

Las cotas suficientes de RM tampoco certifican este conjunto: Liu–Layland da
0,828427 y aquí `U=0,971429`; la prueba hiperbólica da 2,2, mayor que 2. Que
estas pruebas suficientes no pasen **no demuestra por sí mismo** que RM falle;
RTA y la ejecución son la evidencia del fallo de `τ2`.

**Con EDF**, la tarea lista con deadline más próximo recibe la CPU. Para tareas
independientes con `D=T`, una utilización `U≤1` es necesaria y suficiente en
el modelo ideal. La ejecución real de 60 s en la ESP32-S3 informó cero misses
para ambas tareas. La captura
de WaveForms muestra pulsos cercanos a `C1=2 ms` y `C2=4 ms`; el contador
serial de 60 s es el que respalda el resultado de cero incumplimientos.

### B. Inversión de prioridad: semáforo y mutex

Se usaron tres hilos de prueba: `control` alta (prioridad 2), `medium` (4) y
`low` (7). Low toma un lock; luego control intenta tomarlo y queda esperando;
medium se libera mientras low todavía tiene el lock.

| Caso | Respuesta de control | Low sostiene lock | Trabajo de medium | Lectura |
|---|---:|---:|---:|---|
| Sin contención | 0,93830 ms | — | — | Línea base aislada de Task B. |
| Semáforo como lock | 50,466 ms | 51,604 ms | 23,444 ms | Medium preempta a low; control sigue bloqueado. |
| Mutex con herencia | 27,001 ms | 28,148 ms | 23,442 ms | Low hereda prioridad alta y acaba antes de que medium lo preempte. |

El semáforo no sabe que una tarea de prioridad alta está esperando, así que
medium puede interrumpir a low. Con `k_mutex`, la herencia de prioridad eleva
temporalmente a low y reduce la espera de control. La respuesta baja unos
23,465 ms frente al caso del semáforo, pero **27,001 ms todavía excede el
deadline de control de 10 ms**. La sección crítica de este experimento es
demasiado larga para ese plazo.

Con `C_control=0,93830 ms`, la envolvente `C+B` da 52,54230 ms para el caso del
semáforo y 29,08630 ms para el mutex. Las respuestas medidas fueron menores,
pero todavía por encima de 10 ms. Son envolventes basadas en `B` observado, no
pruebas de WCET. Task B es una demostración aislada de inversión; esos números
no son el `C_i` de control del nodo SoilSense.

### C. `calib`: CPU y tiempo de pared

Antes, `calib` hacía 1000 repeticiones de `k_busy_wait(400)`. Cada espera
ocupada dura unos 400 μs y retiene la CPU; la estimación es cerca de 400 ms de
CPU y se midieron 400,8 ms de pared.

Después se usaron 1000 `k_usleep(400)`. Dormir libera CPU para otros hilos: la
estimación indirecta de CPU fue 2,804 μs × 1000 = 2,804 ms, mientras que el
tiempo de pared fue 499,4 ms. El tiempo de pared es mayor por redondeo al tick
y planificación; no contradice que la CPU ocupada haya bajado mucho. La cifra
2,804 ms es una estimación basada en la traza Week 5, no una lectura directa de
un contador ni WCET. Además es una **cota baja**: los 2,804 μs por ronda cubren
la lectura y el bucle entre esperas, pero no el costo de dormir y despertar el
hilo en cada `k_usleep` (llamada al kernel, programar el timeout y dos cambios
de contexto). La CPU real después del arreglo es algo mayor que 2,804 ms,
aunque sigue muy por debajo de los ≈400 ms de antes.

### D. E-stop antes y después

Antes del arreglo, el evento de seguridad esperaba a que el control periódico
revisara la condición (hasta unos 10 ms) y al siguiente paso PWM (hasta 1 ms).
La estimación era hasta unos 11 ms. En cinco pruebas, el máximo observado fue
10,080 ms, que supera el límite de 5 ms.

Después, sampling detecta la condición y baja la salida PWM dentro de esa misma
ruta; ya no espera al siguiente control. Una prueba midió 0,2561 μs entre el
marcador de detección y la bajada de la válvula. Es un solo ensayo, no un máximo
ni WCET. La arquitectura mejora la ruta observada, pero hace falta más prueba
para afirmar una cota.

### E. Anchos observados en las rutas S3

AD2 midió anchos de pulsos GPIO para estimar cuánto duraron rutas instrumentadas
en la ESP32-S3. El ancho del marcador es `C_obs` de esa ruta bajo esa prueba; no
es por sí solo la respuesta completa de extremo a extremo ni WCET.

| Ruta | Observación | Qué limita la conclusión |
|---|---|---|
| Sampling, DIO0 | Media 0,93787 μs; p99 1,000 μs; máx 4,200 μs (`n=3000`). | Máximo observado en ventana de 3 s. |
| Control, DIO1 | Media 351 ns; p99/máx 400 ns (`n=300`). | No confundir con Task B aislada ni con período de 10 ms. |
| Telemetría, DIO3 | Media 63,967 μs; p99/máx 64 μs (`n=3`). | Muy pocas muestras; conteo preliminar. |
| Consola `status`, DIO2 | Ocho pulsos; media 10,625 μs; p99/máx 69,1 μs. | Comando esporádico; no tiene deadline de llegada asignado. |
| Lote de flow, DIO4 | Tres lotes; media 2,467 μs; p99/máx 2,5 μs. | 300 bordes de entrada a 100 Hz produjeron tres lotes. |
| Display, DIO5 | Cinco pulsos; media 24,630 ms; máx 24,641 ms. | OLED no verificada visualmente; excluir de `C_i` validado. |

### F. Qué análisis de tareas quedó abierto

RTA independiente para sampling, control y telemetría usó los máximos observados
como entradas (`C_obs`), no WCET: utilizaciones/períodos asumidos dan
`U=0,004304` (0,4304 %) para ese subconjunto. Con esas entradas calculó
`R_sampling=0,0042 ms`, `R_control=0,0046 ms` y
`R_telemetry=0,0686 ms`, todos por debajo de los deadlines asumidos. El cálculo
pasa bajo ese modelo,
pero no cubre el nodo entero. Faltan parámetros de las tareas de Daniela,
modelo de llegada/deadline de consola y flow y validación del display. Tampoco
se incluyó bloqueo ni jitter de liberación. Por tanto, no presentar el resultado
del subconjunto como certificación del sistema SoilSense completo.

El display es la omisión que más pesa. Con su máximo observado de 24,641 ms
cada 500 ms, solo él aportaría `24,641/500 ≈ 0,049` (≈4,9 %) a la utilización:
más de diez veces el 0,43 % del subconjunto calculado. Si se valida el OLED,
el `U` del nodo pasaría a rondar el 5,4 %. Sigue siendo bajo, pero cambia el
orden de magnitud y es la tarea que más tiempo de CPU ocuparía.

### Conclusión y exposición sugerida

La Semana 6 mostró que la política de planificación importa (RM incumple en
Fig. 4.13 y EDF no en la ejecución de 60 s en la S3), que un semáforo usado como
lock puede causar inversión de prioridad y que un mutex con herencia reduce esa
inversión, aunque una sección crítica demasiado larga sigue rompiendo el plazo.
También se reemplazó una espera ocupada de `calib` por esperas que ceden CPU y
se movió la respuesta del E-stop a sampling. Las mediciones S3 caracterizan
rutas, pero siguen siendo observaciones limitadas; no demuestran WCET ni el
cumplimiento del conjunto completo de tareas.

> “En la Semana 6 estudiamos planificabilidad con RM y EDF, analizamos los
> tiempos de respuesta y probamos inversión de prioridad. RM hizo fallar a
> `τ2`; EDF no registró fallas en 60 segundos para la carga de la Fig. 4.13.
> Vimos que un semáforo como lock permite que la tarea media retrase al control,
> mientras que el mutex con herencia reduce esa inversión, aunque la sección
> crítica seguía siendo demasiado larga para 10 ms. Luego cambiamos `calib`
> para que durmiera en vez de ocupar CPU y atendimos el E-stop desde sampling.
> Las capturas S3 miden anchos observados de varias rutas; como faltan WCET y
> parámetros de algunas tareas, todavía no prueban el nodo completo.”

### Cómo explicar cada bloque de la tabla de predicciones/resultados

La columna **Predicción** dice qué esperábamos antes de medir; **Base** es el
razonamiento/modelo; **Valor observado** es lo que mostró la prueba; y
**Referencia** identifica el archivo que respalda ese dato. Una predicción y
una observación pueden describir magnitudes distintas: hay que nombrar qué se
midió.

#### 1. RM y EDF

- **RM:** el modelo predice que `τ2` incumple a su deadline absoluto de 7 ms,
  porque RTA da 8 ms. En 60 s se contaron 3429 incumplimientos de `τ2`. El
  máximo de 6,0099 ms es el ancho del marcador DIO1, no la respuesta desde la
  liberación. En el ejemplo, `τ2` se libera en `t=0`, pero empieza a ejecutar
  en `t=2`; su marcador puede quedar alto de `t≈2` a `t≈8` (unos 6 ms) y aun
  así terminar un milisegundo después del deadline `t=7`. El contador confirma
  los misses; el pulso ilustra la ejecución interrumpida.
- **EDF:** `U=0,971429≤1` predice cero misses para las dos tareas bajo el modelo
  ideal (independientes y `D=T`). La ejecución de 60 s observó cero en `τ1` y
  `τ2`. Es un resultado para Fig. 4.13, no una prueba EDF del nodo SoilSense.

#### 2. Respuesta de control e inversión

- **Línea base aislada:** el pulso de control de Task B midió 0,93830 ms sin
  lock, hilo low ni medium. La estimación previa de unos 12 μs venía de un
  traspaso GPIO de Week 4; ese traspaso no era el tiempo completo de esta ruta.
  Por eso no se comparan como si fueran la misma métrica. El 0,93830 ms tampoco
  es el `C_i` de control del firmware SoilSense en la S3.
- **`k_sem`:** control (alta) espera el semáforo tomado por low (baja); medium
  puede interrumpir a low y extender la espera. Se observaron respuesta de
  control 50,466 ms, low con el lock 51,604 ms y medium activo 23,444 ms. El
  ancho de low es duración de lock observado en esa ejecución e incluye la
  interferencia de medium; no es `B` máximo garantizado por el semáforo.
- **`k_mutex`:** la herencia de prioridad permite que low eleve temporalmente
  su prioridad y termine antes de que medium lo interrumpa. Se midieron
  respuesta de control 27,001 ms y `B` de 28,148 ms. Mejoró respecto al
  semáforo, pero la respuesta aún excedió el deadline de 10 ms. Heredar
  prioridad reduce inversión; no acorta automáticamente una sección crítica
  larga.

#### 3. Calibración y E-stop

- **`calib` antes:** 1000 `k_busy_wait(400)` piden unos 400 ms de CPU. El GPIO
  midió 400,8 ms de pared. La cifra de CPU se estima por las 1000 esperas; no
  es una lectura independiente de ciclos.
- **`calib` después:** 1000 `k_usleep(400)` ceden CPU. La duración de pared fue
  499,4 ms; la estimación indirecta de CPU fue 2,804 ms a partir de la traza
  Week 5. El tiempo de pared puede crecer por el tick/planificación aunque el
  uso de CPU caiga. La estimación de CPU no es directa ni WCET, y es una cota
  baja: no incluye el costo de dormir y despertar el hilo en cada `k_usleep`.
- **E-stop antes:** el camino podía esperar al control (hasta unos 10 ms) y al
  paso PWM (hasta 1 ms). Se observaron 10,080 ms de máximo en cinco pruebas,
  por encima del requisito de 5 ms.
- **E-stop después:** al bajar la válvula en la ruta de sampling se observó
  0,2561 μs en un solo ensayo. Es una mejora observada, no máximo, garantía ni
  WCET; hacen falta más viajes y una cota de la ruta.

#### 4. Anchos de pulso de las rutas S3

Estas filas no son “períodos” de las tareas. Muestran el ancho del marcador
GPIO alrededor de una ruta y se usan como `C_obs` aproximado. `n` es cuántos
anchos se observaron; **p99** es el percentil 99: 99 % de las muestras quedó
en o por debajo de ese valor. Ningún máximo de una ventana corta equivale a
WCET.

- **Sampling DIO0:** media 0,93787 μs, p99 1 μs, máximo 4,2 μs en 3000 pulsos.
  Es ancho instrumentado de la ruta de muestreo; su liberación ocurre cada
  1 ms, magnitud distinta.
- **Control DIO1:** media 351 ns y p99/máximo 400 ns en 300 pulsos. Es el
  marcador de la ruta periódica; no el ensayo de inversión de Task B ni el
  período de control de 10 ms.
- **Telemetría DIO3:** media 63,967 μs y máximo 64 μs, pero solo `n=3` en 3 s.
  La muestra es preliminar por el bajo conteo.
- **Consola `status` DIO2:** ocho pulsos, media 10,625 μs y máximo 69,1 μs.
  Es esporádica y no tiene deadline de llegada especificado.
- **Lote de flow DIO4:** 300 flancos de entrada a 100 Hz produjeron tres lotes;
  el marcador de procesamiento tuvo media 2,467 μs y máximo 2,5 μs. Eso mide
  duración de lote, no latencia desde cada flanco hasta servirlo.
- **Display DIO5:** cinco pulsos en 3 s; ancho medio 24,630 ms y máximo
  24,641 ms. La actualización física del OLED no se validó visualmente, así
  que se conserva el dato pero no se usa como `C_i` validado.

Las referencias `EV-W06-...` indican dónde consultar el registro o captura;
los CSV son exportaciones de anchos/eventos y los PNG muestran la interfaz del
analizador. El reporte de RM/EDF incluye además el contador serial, porque esa
es la fuente de los misses a 60 s.

#### Imagen 1: `EV-W06-RM-001.png` — traza de τ1 y τ2 bajo RM

La captura de WaveForms es una ventana ampliada de Logic / Record: 30 M
muestras a 10 MHz (3 s totales, con una vista de 2 ms/div). DIO0/GPIO4 es el
marcador de `τ1`; DIO1/GPIO5 es el de `τ2`. La tabla de Measurements está
configurada para el ancho positivo máximo de DIO1 y muestra **6,0099 ms**.
Ese ancho indica cuánto tiempo de pared estuvo activo el marcador de τ2 en la
muestra más larga. Si la señal permanece activa cuando τ2 es preemptada, ese
ancho incluye la pausa; pero la captura no tiene una señal separada de
preempción ni un marcador de deadline, así que no permite señalar visualmente
el instante exacto de la interrupción.

En el modelo, τ2 se libera en t=0, espera a τ1 hasta t=2, ejecuta 3 ms hasta
t=5, se interrumpe por τ1 entre t=5 y t=7 y ejecuta el último milisegundo
hasta t≈8. El deadline era t=7. Según el modelo y el significado del marcador,
el ancho de unos 6 ms puede coexistir con una respuesta de 8 ms desde la
liberación y con el incumplimiento; la captura no muestra por sí sola toda esa
secuencia de planificación. Tampoco muestra los 3429 misses; ese contador viene del resumen serial guardado en
`EV-W06-RM-001.md`.

#### Imagen 2: `EV-W06-EDF-001.png` — señales de τ1 y τ2 con EDF

DIO0/GPIO4 representa `τ1`; DIO1/GPIO5 representa `τ2`. La captura lógica
contiene 30 M muestras a 10 MHz (3 s de datos), mientras que la ejecución de
la prueba duró 60 s. En Measurements, los anchos positivos son 2,0011 ms para
DIO0 y 4,0016 ms para DIO1: corresponden aproximadamente a los `C` de 2 ms y
4 ms. Las lecturas puntuales de período son 4,9781 ms y 7,4671 ms,
aproximadamente los períodos nominales de 5 ms y 7 ms; la lectura de período
de DIO1 se considera puntual en esta vista.

La captura ayuda a reconocer los dos trabajos y sus anchos/ritmos, pero no
contiene un marcador de deadline ni resume por sí sola los 60 s sin fallas.
El resultado `tau1_misses=0`, `tau2_misses=0` proviene del contador serial de
la ejecución completa. EDF decide cuál hilo corre según el deadline absoluto
más próximo; los pulsos de GPIO no codifican ese deadline directamente.

#### Imagen 3: `EV-W06-INV-BASELINE-001.png` — control sin contención

La prueba usa el firmware independiente `firmware/week06_inversion/` en la
ESP32-S3. DIO1/GPIO5 marca el pulso de control; DIO0 (low) y DIO2 (medium)
permanecen inactivos, señal de que en este caso aislado no hay competencia por
el lock ni trabajo de medium. WaveForms mide `PosWidth = 0,93830 ms` para DIO1.
Esa es la respuesta/ancho de la ruta de control en esta prueba de referencia.

La expectativa de unos 12 μs venía de medir un traspaso de tareas de la
Semana 4; no era una predicción del ancho completo de control. Por eso no se
comparan como si fueran la misma métrica. Este baseline sirve como `C`
observado para analizar después el ejemplo aislado de inversión (Task B), pero
no es el `C_i` medido dentro del nodo SoilSense completo.

#### Imagen 4: `EV-W06-INV-SEMAPHORE-001.png` — inversión con semáforo

La lectura de niveles es distinta para cada canal:

| Canal | Señal alta significa | Ancho medido |
|---|---|---:|
| DIO0 / GPIO4 | `low` tiene tomado el lock; no implica que `low` esté ejecutándose en ese instante. | 51,604 ms |
| DIO1 / GPIO5 | Está abierto el intervalo de respuesta de `control`, incluida su espera bloqueada; no significa que `control` use la CPU todo ese tiempo. | 50,466 ms |
| DIO2 / GPIO6 | `medium` está ejecutando el trabajo marcado. | 23,444 ms |

La secuencia visible es: sube DIO0 cuando low toma el lock; sube DIO1 cuando
control lo solicita y empieza su intervalo de respuesta; después sube DIO2 y
medium ejecuta mientras DIO0 y DIO1 continúan altos. Cuando DIO2 baja, medium
terminó, pero low aún conserva el lock. Luego low lo libera (baja DIO0) y
control termina su respuesta (baja DIO1). Los tres niveles altos se solapan
porque marcan estados/intervalos distintos; no significan que los tres hilos
usen la CPU a la vez. Medium retrasa indirectamente a control porque control
depende del lock de low. El `k_sem` no aplica herencia de prioridad. Estos
anchos son de Task B, no WCET ni mediciones del nodo completo.

#### Imagen 5: `EV-W06-INV-MUTEX-001.png` — herencia de prioridad con mutex

Los canales tienen el mismo significado que en la imagen de `k_sem`: DIO0
marca que low conserva el lock, DIO1 el intervalo de respuesta de control
(incluida espera) y DIO2 la ejecución de medium. Aquí los anchos medidos son
**28,148 ms** para low/lock, **27,001 ms** para la respuesta de control y
**23,442 ms** para medium.

La diferencia visual clave es que DIO2 empieza después de que DIO0 baja y
DIO1 termina: medium no interrumpe a low mientras este conserva el mutex.
Cuando control queda bloqueado en el mutex, Zephyr eleva temporalmente la
prioridad de low para que termine la sección crítica y libere el recurso.
Después corre control; medium corre más tarde. Esa es la herencia de prioridad.
La respuesta de control baja de 50,466 ms con semáforo a 27,001 ms con mutex,
pero todavía supera el deadline de 10 ms. La herencia reduce la inversión; la
sección crítica aún es demasiado larga.

#### Imagen 6: `EV-W06-CALIB-BEFORE-001.png` — `calib` antes del arreglo

Esta captura distingue dos cantidades que no deben confundirse:

- **Tiempo de CPU estimado:** antes del arreglo, `calib` ejecutaba 1000
  iteraciones de `k_busy_wait(400)`. Cada espera ocupada solicitaba 400 µs de
  CPU; por tanto, `1000 × 400 µs = 400 ms` de CPU estimada, además del costo
  del bucle y de las lecturas. Es una estimación derivada del código, no una
  lectura directa de un contador de ciclos.
- **Tiempo de pared medido:** DIO3/GPIO6 (`instr_cons`) se mantiene alto
  mientras se ejecuta `calib`. En la captura, los cursores marcan X1 = 292,2 ms
  y X2 = 693 ms; la diferencia, ΔX = 400,8 ms, es el tiempo transcurrido desde
  el inicio hasta el fin del comando. Incluye todo lo que ocurrió durante ese
  intervalo, no solo los ciclos de CPU de `calib`.

La duración de pared, 400,8 ms, es coherente con los aproximadamente 400 ms de
esperas ocupadas: `k_busy_wait` consume CPU en vez de dormir y cederla. La
captura mide directamente la duración de pared; la cifra de CPU se infiere del
código. No es una medición de WCET ni demuestra que el comando siempre tarde
exactamente lo mismo.

**Cómo explicarlo:** “Antes del arreglo, `calib` repetía mil esperas ocupadas
de 400 microsegundos, unas 400 milésimas de segundo de CPU solicitada. El AD2
mide por el pulso DIO3 que el comando duró 400,8 ms de tiempo de pared. Como la
espera ocupada no cede el procesador, casi todo ese intervalo se emplea
manteniendo ocupado el CPU.”

#### Imagen 7: `EV-W06-CALIB-AFTER-001.png` — `calib` después del arreglo

Después del cambio, las 1000 esperas ocupadas se sustituyeron por
`k_usleep(400)`. La diferencia conceptual es que `k_busy_wait` mantiene el
subproceso consumiendo CPU, mientras que `k_usleep` lo suspende para que el
planificador pueda ejecutar otros trabajos.

En la captura, DIO3/GPIO6 (`instr_cons`) vuelve a señalar la duración completa
del comando. Los cursores muestran X1 = 328,4 ms y X2 = 827,8 ms; por tanto,
ΔX = 499,4 ms de **tiempo de pared**. El comando tarda más que antes en
terminar, aunque ya no ocupa continuamente el procesador durante ese intervalo.
Los pulsos que continúan en DIO1 y DIO2 son compatibles con que otras tareas
siguen ejecutándose durante la calibración.

Los ≈2,804 ms de **CPU** no se leen en esta captura. Son una estimación
indirecta: la traza CTF de la semana 5 mostró unos 2,804 µs entre una espera y
la siguiente; multiplicados por las 1000 rondas dan aproximadamente 2,804 ms.
La estimación depende de que ese intervalo represente el trabajo de CPU entre
esperas; no es una lectura directa de un contador de ciclos ni un WCET. Con
`k_usleep` cada ronda suma, además, el costo de suspender y reanudar el hilo,
que la traza de la Semana 5 (hecha con `k_busy_wait`) no contiene; por eso
2,804 ms es una cota baja de la CPU real.

Así se resuelve la aparente paradoja: **después, `calib` tarda unos 499,4 ms
de pared, pero la estimación de CPU baja de unos 400 ms a unos 2,804 ms**. El
resto del tiempo transcurre principalmente mientras el hilo está dormido y
cede el procesador. El resultado reduce la interferencia con las tareas
periódicas a cambio de que el comando tarde más en completarse.

**Cómo explicarlo:** “Después del arreglo cambiamos las esperas ocupadas por
esperas dormidas de 400 microsegundos. El GPIO mide 499,4 ms de tiempo de pared
para `calib`, pero la traza permite estimar solo unos 2,804 ms de CPU para las
mil rondas. La captura mide directamente el tiempo de pared; el CPU es una
estimación indirecta.”

#### Imagen 8: `EV-W06-ESTOP-BEFORE-001.png` — e-stop antes del arreglo

La prueba mide cuánto pasa desde que el sistema detecta la condición de
emergencia hasta que la señal que manda cerrar la válvula baja:

- **DIO0/GPIO15 sube:** marca la detección del e-stop por `sampling`.
- **DIO1/GPIO21 baja:** marca que la salida de válvula ya fue llevada a nivel
  bajo.
- Los cursores de esta captura muestran aproximadamente X1 = −32,0102 µs y
  X2 = 10,05 ms; la diferencia indicada es **ΔX = 10,08 ms**. Es una de las
  cinco pruebas y corresponde al máximo observado de **10,080 ms**.

El mecanismo anterior no bajaba la válvula en el mismo trabajo que detectaba
la emergencia. `sampling` señalaba el estado y el control periódico debía
recogerlo en su siguiente ejecución para actualizar la salida. Como el control
se libera cada 10 ms, la espera podía acercarse a un período completo; además,
se consideró hasta 1 ms para que el siguiente paso de PWM reflejara el cambio.
Así se obtuvo la estimación conservadora de aproximadamente **10 ms + 1 ms =
11 ms**.

La cifra de 11 ms es el cálculo del camino esperado; **10,080 ms es el máximo
medido en cinco viajes**. No tienen que ser idénticos: la captura observó una
demora concreta, no todos los posibles alineamientos entre detección, control
y PWM. Ambos resultados superan el requisito de 5 ms, por lo que el mecanismo
anterior falla. La captura permite leer el intervalo entre los dos marcadores;
no contiene por sí misma una línea de umbral de 5 ms: el veredicto sale de
comparar ΔX con el requisito.

**Cómo explicarlo:** “Antes del cambio, el e-stop se detectaba en sampling,
pero la válvula esperaba a que el control periódico actualizara su salida.
Ese camino podía esperar hasta unos 10 ms por el siguiente control y hasta
1 ms por el PWM. En cinco pruebas, el máximo medido fue 10,080 ms, superior al
límite de 5 ms.”

#### Imagen 9: `EV-W06-ESTOP-AFTER-001.png` — e-stop después del arreglo

En esta captura DIO0/GPIO15 sube al marcar la detección y DIO1/GPIO21 baja al
ordenar la válvula cerrada. Los flancos se ven prácticamente alineados porque
la vista está a **1,8 ms/div**; una separación de **0,2561 μs** es demasiado
pequeña para distinguirla a esa escala. La cifra exacta viene de la medición
registrada para un viaje, no de estimar píxeles en la imagen.

La cercanía de los flancos ilustra que, después del cambio, la salida baja
dentro del camino de sampling sin esperar la siguiente activación de control.
El registro informa **0,2561 μs en un solo ensayo**, menor que el requisito de
5 ms; no es un máximo repetido ni una garantía WCET.

### Rutas instrumentadas del nodo S3: cómo explicar las últimas filas

Estas cinco filas son **anchos positivos de pulsos GPIO** capturados con el
AD2. El ancho aproxima cuánto duró la ruta marcada durante esa observación;
no es el período de activación, la latencia desde un evento externo ni una
cota WCET. Sampling y control tienen muchas observaciones; telemetría,
consola, flow y display tienen pocos eventos o activaciones dirigidas, así que
sus conclusiones son más limitadas.

| Ruta | Pin/señal | Qué representa el pulso | Resultado y lectura |
|---|---|---|---|
| Control, 100 Hz | DIO1 / GPIO5, `instr_ctrl` | Ejecución instrumentada de la ruta periódica de control. | 300 pulsos: media 351 ns, p99 y máximo 400 ns. Es el ancho observado; el período nominal de 10 ms es otra magnitud. |
| Telemetría, 1 Hz | DIO3 / GPIO7, `instr_tele` | Publicación de telemetría; el pulso incluye `printk` por UART polling. | Solo 3 pulsos: media 63,967 μs, máximo 64 μs. Preliminar por el tamaño de muestra. “1 Hz” es la frecuencia de la tarea; la captura del AD2 fue a 10 MHz. |
| Consola `status` | DIO2 / GPIO6, `instr_cons` | Actividad instrumentada mientras se procesa el comando; pueden aparecer varios pulsos asociados a recepción y manejo. | Un comando produjo 8 pulsos: media 10,625 μs, máximo 69,1 μs. No implica ocho comandos ni mide la respuesta completa; faltan deadline y modelo de llegadas. |
| Lote de flow | DIO4 / GPIO15, `instr_flow` | Trabajo de procesamiento del lote, no cada interrupción de entrada. | DIO6/GPIO16 aplicó 300 flancos a 100 Hz; cada 100 flancos se generó un lote. Se observaron 3 pulsos: media 2,467 μs, máximo 2,5 μs. No mide la latencia desde cada flanco hasta el servicio. |
| Display, refresco esperado cada 500 ms | DIO5 / GPIO17, `instr_disp` | Intento de refresco en la variante compilada con OLED. | 5 pulsos en 3 s: media 24,630 ms, máximo 24,641 ms. El OLED no mostró imagen; dato no validado y excluido del `C_i` aceptado. |

#### Control S3 — `EV-W06-CI-S3-DIO1-events.csv`

WaveForms contó **300 pulsos positivos** en una ventana de 3 s a 10 MHz. La
media fue **351 ns** y el p99/máximo **400 ns**. Esto caracteriza la duración
del marcador de control bajo esa carga. No significa que control se active
cada 351 ns: su período nominal es **10 ms**, equivalente a 100 Hz. Tampoco
es el resultado de la demostración de inversión de prioridad, que usó otro
firmware.

#### Telemetría S3 — `EV-W06-CI-S3-DIO3-events.csv`

Se observaron **3 pulsos** en 3 s, coherentes con la ruta esperada de 1 Hz. La
media fue **63,967 μs** y el máximo **64 μs**. DIO3/GPIO7 cubre la ruta de
telemetría y su salida `printk` por UART polling. Tres observaciones sirven
como caracterización inicial, pero son pocas para establecer una distribución
estable o una cota máxima.

**Por qué es tan distinto de la NUCLEO (5,777 ms):** en la Semana 2 la misma
ruta de telemetría duró casi 90 veces más. Una explicación probable, **no
verificada**, es que la UART de la ESP32-S3 tiene un FIFO de transmisión por
hardware (128 B) que acepta la línea CSV completa de una vez; el `printk`
termina al llenar el FIFO y los caracteres salen después por el cable. En la
NUCLEO, con UART por *polling*, el código espera a que salga cada carácter a
115200 baudios (≈87 μs por carácter), así que una línea de unos 60 caracteres
ya cuesta ≈5 ms. Para confirmarlo habría que medir el pin TX de la UART junto
con el marcador DIO3. Presentarlo como hipótesis.

#### Consola `status` S3 — `EV-W06-CI-S3-DIO2-events.csv`

Durante una captura de 3 s se envió un comando `status`. DIO2/GPIO6 registró
**8 pulsos**, con media **10,625 μs** y máximo **69,1 μs**. Los pulsos marcan
actividad del camino instrumentado de consola; no son necesariamente ocho
comandos. El máximo describe un tramo observado, no el tiempo desde que el
usuario envía el comando hasta que recibe toda la respuesta. Como el comando es
esporádico y su deadline todavía no está especificado, estos datos no bastan
para juzgar su plazo.

#### Flow S3 — `EV-W06-CI-S3-DIO4-events.csv`

Para generar trabajo reproducible, DIO6/GPIO16 aplicó pulsos de entrada a
**100 Hz** durante 3 s: **300 flancos** en total. La lógica agrupa cada 100
flancos y ejecuta un lote; por eso DIO4/GPIO15 produjo **3 pulsos de lote**.
Cada pulso mide el tiempo de ejecución del procesamiento del lote: media
**2,467 μs**, máximo **2,5 μs**. No mide la latencia ISR-a-servicio de cada
flanco ni un deadline de flow, que todavía no está asignado.

#### Display S3 — `EV-W06-CI-S3-periodic-003.csv`

La variante de display intentó refrescar cada **500 ms**. En la captura de 3 s
se registraron **5 pulsos** DIO5/GPIO17; el ancho medio fue **24,630 ms** y el
máximo **24,641 ms**. Sin embargo, el OLED no mostró una imagen. La captura
confirma que se ejecutó una ruta marcada, pero no demuestra que la pantalla
funcionara ni que el pulso corresponda a un refresco visual correcto. Por eso
el RET conserva el dato como no validado y no lo usa como `C_i` aceptado.

**Cómo presentar estas cinco filas:** “En la S3 medimos anchos de rutas GPIO,
no períodos ni WCET. Control tuvo 300 muestras y un máximo observado de 400 ns;
telemetría tuvo solo tres muestras, de hasta 64 μs. Un `status` produjo ocho
pulsos de hasta 69,1 μs, pero no tenemos aún su deadline. Tres lotes de flow,
formados con 300 flancos de entrada, midieron hasta 2,5 μs cada uno. El display
generó cinco pulsos de hasta 24,641 ms, pero como el OLED no mostró imagen, no
aceptamos ese valor como ejecución funcional validada.”

### Preguntas de comprobación

1. Si en la Fig. 4.13 `U=0,971≤1`, ¿por qué RM falla y EDF no? ¿Qué dice cada
   prueba (Liu–Layland, hiperbólica, RTA) y cuál decide?
2. ¿Por qué el mutex con herencia de prioridad redujo la respuesta de control
   de 50,466 ms a 27,001 ms, pero aun así no cumple los 10 ms?
3. Después del arreglo, `calib` tarda más en tiempo de pared (499,4 ms frente a
   400,8 ms). ¿Por qué eso es una mejora?
4. ¿Qué hace falta para afirmar que el e-stop cumple 5 ms, además del único
   ensayo de 0,2561 μs?

## Síntesis general de los laboratorios

| Semana | Qué se hizo | Qué se aprendió / resultado principal |
|---|---|---|
| 1 | Se preparó el entorno Zephyr; se compiló y ejecutó `hello_world` en `native_sim`, y `blinky` en la NUCLEO-G474RE. | Se verificaron compilación, programación, GPIO y consola. Aún no se midió temporización de SoilSense. |
| 2 | Se instrumentó y midió el superbucle en la NUCLEO-G474RE, con caché activa/desactivada y durante `calib`. | El muestreo llegó a 6,845 ms de período máximo en reposo y 414,06 ms con `calib`; el backlog llegó a 414 ticks. El trabajo síncrono del superbucle podía bloquear el muestreo. |
| 3 | Se llevó el firmware a ESP32-C6 y se movió el muestreo a un hilo prioritario. | Con `calib`, el jitter de muestreo cayó de 402,47 ms a 2 μs y el backlog a cero; su período máximo fue 1,002 ms. Control siguió bloqueado en `main` y alcanzó 412,31 ms. |
| 4 | Se migraron las rutas restantes a hilos y objetos del kernel Zephyr; se midió el costo y el uso de pila. | Con `calib`, el jitter máximo observado de muestreo fue 6,2 μs y el período máximo de control 10,003 ms. Fue una mitigación grande, pero todavía se excedieron ligeramente los períodos nominales. |
| 5 | Se activó CTF en ESP32-S3 para reconstruir el orden de ejecución y comparar la temporización con y sin trazado. | El jitter absoluto máximo fue 12,7 μs con CTF y 2,3 μs sin CTF: diferencia observada de 10,4 μs. La traza explica quién corrió; el AD2 mide la señal física sin instrumentación CTF. |
| 6 | Se compararon RM/EDF, se probó inversión de prioridad con semáforo/mutex, se corrigieron `calib` y el e-stop, y se midieron rutas GPIO del S3. | RM incumplió para `τ2`; EDF no tuvo misses en 60 s para el modelo de la guía. El mutex redujo la inversión, pero no alcanzó el deadline de 10 ms. La corrección del e-stop midió 0,2561 μs en un viaje. Las rutas S3 están caracterizadas de forma parcial. |

## Conclusión global

La evolución principal fue pasar de un **superbucle donde una operación
síncrona de consola podía retrasar el muestreo y el control por cientos de
milisegundos** a una arquitectura Zephyr con tareas separadas y prioridades.
Las mediciones muestran una reducción marcada de esa interferencia. Después,
el análisis de Semana 6 permitió distinguir entre mejorar el comportamiento
observado y demostrar formalmente un plazo.

La evidencia hasta la Semana 6 **no prueba todavía que todo el nodo SoilSense
sea hard real-time**. Los períodos máximos observados no son WCET; algunas
mediciones tienen pocas muestras; la ruta del display no fue validada; faltan
modelos de llegada/deadline para ciertas tareas y los parámetros de las tareas
de Daniela. La conclusión rigurosa es: hubo mejoras arquitectónicas y algunas
rutas se midieron, pero la demostración formal del sistema completo sigue
pendiente.

> “Empezamos verificando el entorno y midiendo el superbucle. Allí encontramos
> que `calib` bloqueaba el procesamiento y podía retrasar el muestreo cientos
> de milisegundos. Separamos tareas usando Zephyr y las mediciones mostraron
> mucha menos interferencia, aunque todavía hubo pequeños excesos frente a
> ciertos períodos nominales. Luego estudiamos trazado, planificación,
> inversión de prioridad y seguridad. Las correcciones mejoraron las rutas
> observadas, pero el RET aún no demuestra WCET ni la planificabilidad del nodo
> completo.”
