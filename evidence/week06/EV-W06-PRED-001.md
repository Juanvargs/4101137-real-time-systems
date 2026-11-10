# EV-W06-PRED-001 — Predicciones previas al laboratorio

**Fecha:** 2026-11-04  
**Estado:** registradas antes de las mediciones de Week 6.  
**Plataforma prevista:** ESP32-S3 DevKitC.

| Medición de la guía | Predicción previa | Fundamento |
|---|---|---|
| Fig. 4.13 con RM | `τ2` pierde su deadline en `t = 7 ms` | En `t=5 ms`, `τ1` libera otra activación y, por mayor prioridad RM, preempta a `τ2`, que conserva 1 ms de ejecución. |
| Fig. 4.13 con EDF durante 60 s | 0 deadlines perdidos | `U = 2/5 + 4/7 = 0.9714 < 1`; EDF asigna antes la activación de `τ2` con deadline absoluto en `t=7 ms`. |
| Respuesta máxima de control sin contención | Menor que su deadline de 10 ms; número pendiente de obtener `C_i` en la S3 | Sin lock ni inversión, la respuesta será la ejecución de control más la interferencia periódica de sampling; no hay aún un `C_i` de control medido en esta placa. |
| Respuesta máxima de control con inversión y `k_sem` | Mayor que sin contención; puede exceder 10 ms | La tarea media puede preemptar a la tarea baja mientras esta conserva el semáforo, dejando a control bloqueado. El valor depende de las secciones medidas. |
| Respuesta máxima con `k_mutex` | Acotada por `C + B` más interferencia de tareas de mayor prioridad; se espera menor que con `k_sem` | Priority inheritance eleva temporalmente la prioridad del dueño del mutex. Confirmar con `B` medido. |
| Sección crítica larga `B` | La anchura del pulso GPIO será el tiempo con el lock tomado | Medir la anchura en el analizador lógico; no asignar valor hasta la captura. |
| `calib` antes del arreglo: CPU / duración | Cerca de 400 ms / cerca de 400 ms | 1000 rondas × 400 µs de `k_busy_wait(400)` consumen aproximadamente 400 ms de CPU, más el coste de las lecturas y el bucle. |
| `calib` después del arreglo: CPU / duración | CPU mucho menor que 400 ms; duración cercana a 400 ms | `k_usleep(400)` cede CPU. La duración real incluye redondeo al tick de 100 µs y planificación; estimar CPU desde el coste de una ronda medido en la traza Week 5. |
| E-stop antes del arreglo: detección → válvula baja | Puede superar 5 ms; cota de diseño ≈ 11 ms | Tras detectar, control puede tardar hasta 10 ms en ejecutarse; la siguiente actualización PWM puede añadir hasta 1 ms. |
| E-stop después del arreglo: detección → válvula baja | Menor que 1 ms, más la latencia/ejecución de sampling | Si sampling pone `duty_pct=0` al detectar y ejecuta el paso PWM en el mismo job, no espera al siguiente control. |

Las predicciones dependientes de tiempos de ejecución y de la sección crítica quedan expresadas como límites/modelos hasta medir `C_i` y `B`; no se usan números de otra placa como si fueran resultados de la S3.
