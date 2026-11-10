# Week 6 — control de avance

Plataforma: ESP32-S3 DevKitC. Usar GPIO4/GPIO5 para los pulsos de Task A; GPIO21 es la válvula en Task C. No asumir los parámetros de Daniela que se anuncien en clase: registrarlos cuando estén disponibles.

## Antes de medir

- [x] Registrar predicciones sin mezclar estimaciones con resultados: `EV-W06-PRED-001.md`.
- [ ] Confirmar el mapa de GPIO en WaveForms y tener un build limpio de Task A.

## Task A — Fig. 4.13

- [ ] Implementar `τ1 (C=2 ms, T=5 ms)` y `τ2 (C=4 ms, T=7 ms)`, con busy-wait calibrado y un GPIO por tarea.
- [ ] Ejecutar RM y guardar captura que muestre el incumplimiento de `τ2` en `t=7 ms`.
- [ ] Cambiar a EDF: `CONFIG_SCHED_DEADLINE=y`, prioridad estática igual y deadline absoluto establecido antes de bloquear por la siguiente liberación.
- [ ] Ejecutar 60 s, registrar cero/misses y guardar captura EDF.

## Task B — inversión y priority inheritance

- [ ] Medir respuesta de control sin contención.
- [ ] Reproducir low-holds / high-waits / medium-runs usando `k_sem`; medir la demora y guardar captura anotada.
- [ ] Repetir con `k_mutex`; guardar captura anotada.
- [ ] Medir el pulso de la sección crítica del hilo bajo (`B`).
- [ ] Guardar las tres capturas requeridas: sin contención, inversión, mutex PI.

## Task C — corregir `calib` y e-stop en su origen

- [ ] Caso previo: desconectar entrada de flujo; marcar detección con `instr_flow`; medir GPIO21 (válvula) y hacer cinco trips (`set 3300`, luego `set 1500` y `clear`); guardar captura.
- [ ] Medir CPU y duración de `calib` antes del cambio.
- [ ] Cambiar `k_busy_wait(400)` por `k_usleep(400)`; registrar el tick S3 de 100 µs.
- [ ] En sampling, al detectar sobrepresión, poner `duty_pct=0` antes de actualizar PWM.
- [ ] Repetir captura e-stop y medición CPU/duración de `calib`; guardar evidencia.
- [ ] Calcular ajuste temporal de `calib` (CPU disponible sobre 1 s) y sumar el camino e-stop antes/después frente al límite de 5 ms.

## Análisis y entrega

- [ ] Implementar y conservar el script RTA; alimentarlo con los `C_i` medidos y el `B` medido.
- [ ] Calcular utilización, Liu–Layland e hiperbólico para la carga actual y para las dos tareas adicionales cuando sus parámetros estén disponibles.
- [ ] Comparar RTA con mediciones, incluyendo bloqueo.
- [ ] Completar ADR-002 (mutex y `B`) y ADR-005 (arreglos de `calib` y e-stop) con números.
- [ ] Añadir tabla, capturas y resultados a §3 y §4 del RET.
- [ ] Resolver Problem Set (capítulos 2, 4, 7 y RTA), que se entrega en el workshop de Week 7.
- [ ] Revisar evidencias y subir el commit final.
