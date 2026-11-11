# Week 6 — control de avance

Plataforma: ESP32-S3 DevKitC. Usar GPIO4/GPIO5 para los pulsos de Task A; GPIO21 es la válvula en Task C. No asumir los parámetros de Daniela que se anuncien en clase: registrarlos cuando estén disponibles.

## Antes de medir

- [x] Registrar predicciones sin mezclar estimaciones con resultados: `EV-W06-PRED-001.md`.
- [x] Confirmar el mapa de GPIO y compilar la app RM de Task A para la S3.

## Task A — Fig. 4.13

- [x] Implementar `τ1 (C=2 ms, T=5 ms)` y `τ2 (C=4 ms, T=7 ms)`, con busy-wait y un GPIO por tarea.
- [x] Ejecutar RM durante 60 s: `tau1_misses=0`, `tau2_misses=3429` (`EV-W06-RM-001.md`).
- [x] Guardar captura RM y medir DIO1 PosWidth máx. = 6.0099 ms (`EV-W06-RM-001.png`).
- [x] Cambiar a EDF: `CONFIG_SCHED_DEADLINE=y`, prioridad estática igual y deadline absoluto establecido antes de bloquear por la siguiente liberación.
- [x] Ejecutar 60 s sin misses (`tau1_misses=0`, `tau2_misses=0`) y guardar captura EDF (`EV-W06-EDF-001.png` y `.md`).

## Task B — inversión y priority inheritance

- [x] Preparar firmware aislado y repetible para baseline, `k_sem` y `k_mutex`: `firmware/week06_inversion/`.
- [x] Medir respuesta de control sin contención: DIO1 PosWidth = 0.93830 ms (`EV-W06-INV-BASELINE-001.png` y `.md`).
- [x] Reproducir low-holds / high-waits / medium-runs usando `k_sem` con liberaciones temporizadas; control respondió en 50.466 ms, low sostuvo el lock 51.604 ms y medium ejecutó 23.444 ms (`EV-W06-INV-SEMAPHORE-001.png` y `.md`).
- [x] Repetir con `k_mutex`; control respondió en 27.001 ms y medium quedó después de la sección crítica (`EV-W06-INV-MUTEX-001.png` y `.md`).
- [x] Medir el pulso de la sección crítica del hilo bajo: B = 28.148 ms.
- [x] Guardar las tres capturas requeridas: sin contención, inversión y mutex PI.

## Task C — corregir `calib` y e-stop en su origen

- [x] Caso previo: desconectar entrada de flujo; marcar detección con `instr_flow`; medir GPIO21 (válvula) y hacer cinco trips (`set 3300`, luego `set 1500` y `clear`). Valores: 1.024, 2.010, 10.080, 9.987 y 1.012 ms; máximo 10.080 ms (supera el límite de 5 ms).
- [x] Medir `calib` antes del cambio: GPIO6/DIO3 pulso de 400.8 ms; CPU estimada ≈400 ms por 1000 × `k_busy_wait(400)`, más lecturas y bucle.
- [x] Cambiar `k_busy_wait(400)` por `k_usleep(400)`; tick de sistema S3 observado/configurado en 100 µs.
- [x] En sampling, al detectar sobrepresión, poner `duty_pct=0` antes de actualizar PWM.
- [x] Repetir captura e-stop y medición de duración de pared de `calib`: post-arreglo ΔX=256.1 ns; `calib` pared=499.4 ms.
- [x] Corregir la estimación de CPU: se descarta el delta de `console_tid` de ≈115.6 ms porque los reportes `threads` por UART polling dominan el costo; método de la guía estima ≈2.804 ms de CPU (`EV-W06-ESTOP-CALIB-001.md`).
- [x] Sumar presupuesto de un segundo: las tareas del nodo sobre la consola ocupan ≈1.09 % en EV-W05-CPU-001, dejando ≈989 ms/s observados; caben 400 ms pre-fix y ≈2.8 ms post-fix. No es WCET y falta sumar las tareas de Daniela.
- [x] Incluir ambos análisis de suma e-stop en RET y evidencia: antes 10+1≈11 ms; después dispatch de sampling + `C_sampling` restante hasta PWM, microsegundos observados en un trip.

## Análisis y entrega

- [x] Implementar el script RTA y alimentarlo con `C_control=0.93830 ms` y los anchos de bloqueo medidos; separar el `B` empírico del semáforo de la cota por mutex PI (`scripts/week06_rta.py`).
- [x] Calcular utilización, Liu–Layland e hiperbólico para Fig. 4.13. Las tareas extra de Daniela siguen pendientes porque no se han entregado sus parámetros.
- [x] Comparar RTA con los resultados medidos, incluyendo bloqueo (`EV-W06-RTA-001.md`).
- [x] Completar ADR-002 con estado/datos compartidos y ADR-005 con números corregidos y ambas sumas e-stop.
- [x] Añadir tabla predictivo/observado de Semana 6 y lectura en §3.7; registrar por separado qué valores de Task B no son `C_i` del nodo.
- [x] Capturar en la S3 los anchos observados de sampling, control, telemetry, console (`status`) y flow; conservar los CSV de eventos, resumir media/p99/máximo y añadir un RTA parcial de las tres tareas periódicas con períodos y deadlines definidos. Ver `EV-W06-CI-S3-PROCEDURE.md` y `EV-W06-RTA-001.md`.
- [ ] Validar `display`: el build OLED produjo pulsos DIO5 de hasta 24.641 ms, pero la pantalla no mostró imagen. No se acepta ese ancho como `C_i`; el RTA completo del nodo sigue pendiente junto con el modelo de llegada/deadline de console y flow.
- [x] Anotar las tres capturas de Task B con las secuencias de LOW/CONTROL/MEDIUM; conservar los PNG originales y enlazar vistas SVG anotadas.
- [x] Explicar la diferencia entre 3429 misses observados y 1714 del modelo ideal en `EV-W06-RM-001.md`; el mecanismo se presenta como hipótesis consistente con overhead medido/no aislado.
- [x] Hacer que `scripts/week06_rta.py` lea conjuntos modificables desde JSON y permita elegir un conjunto por argumento.
- [x] Declarar el uso de Codex y atribuir las mediciones/capturas a las ejecuciones del usuario.
- Problem Set de capítulos 2, 4, 7 y RTA: omitido por decisión explícita del usuario; el taller de Semana 7 lo solicita como prerrequisito.
- [x] Integrar y subir la documentación/annotaciones y preparar los dos builds del kernel. El commit final del laboratorio depende de capturar los seis `C_i` en la S3 y recalcular el JSON/RTA.
