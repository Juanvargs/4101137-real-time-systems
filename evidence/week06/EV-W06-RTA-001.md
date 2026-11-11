# EV-W06-RTA-001 — Análisis de respuesta y utilización

**Plataforma de las mediciones:** ESP32-S3 DevKitC · **Script:** [`scripts/week06_rta.py`](../../scripts/week06_rta.py) · **Entradas:** [`scripts/week06_rta_input.json`](../../scripts/week06_rta_input.json).

Para repetir el análisis con otros parámetros, edita el JSON y ejecuta
`python scripts/week06_rta.py --input ruta/al/conjunto.json`. Para imprimir solo
un conjunto, agrega `--set "Fig. 4.13 — RM"`. La entrada SoilSense queda marcada
como pendiente hasta medir `C_i`, períodos y deadlines en la S3.

## Fig. 4.13

`τ1: C=2 ms, T=D=5 ms`; `τ2: C=4 ms, T=D=7 ms`.

| Prueba | Resultado | Interpretación |
|---|---:|---|
| Utilización | `U = 2/5 + 4/7 = 0.971429` | El conjunto usa 97.14 % del procesador. |
| Liu–Layland, `n=2` | Límite `0.828427`; `U` lo supera | La prueba suficiente no certifica RM; por sí sola no demuestra fallo. |
| Hiperbólica | `(1+0.4)(1+4/7) = 2.200000 > 2` | La prueba suficiente tampoco certifica RM. |
| EDF | `U ≤ 1` | Pasa para tareas independientes con deadlines implícitos. |
| RTA RM de `τ1` | `R1 = 2 ms ≤ 5 ms` | Cumple. |
| RTA RM de `τ2` | `4 → 6 → 8 → 8 ms`; `R2=8 ms > 7 ms` | Falla; coincide con los misses RM observados durante 60 s. |

La conclusión de RM viene de RTA y de la ejecución, no del fallo de las pruebas
suficientes. La conclusión EDF corresponde al modelo periódico de Fig. 4.13;
no decide si caben los dos sensores de Daniela. Sus `C_i`, períodos y deadlines
no están en los archivos recibidos.

## Task B: control, bloqueo y RTA

Se usa `C_control = 0.93830 ms`, respuesta de control del caso sin contención.
En este experimento no hay tareas periódicas de prioridad superior, así que la
ecuación se reduce a `R = C + B`.

| Escenario | `B` usado | RTA `C+B` | Respuesta medida | `D_control=10 ms` |
|---|---:|---:|---:|---|
| Sin contención | 0 ms | 0.93830 ms | 0.93830 ms | Cumple |
| `k_sem` | 51.604 ms | 52.54230 ms | 50.466 ms | Falla |
| `k_mutex` PI | 28.148 ms | 29.08630 ms | 27.001 ms | Falla |

Los dos valores con contención quedan por debajo de sus envolventes calculadas,
pero exceden el deadline de control de 10 ms. Para `k_mutex`, `B=28.148 ms` es
el máximo medido en esta captura, no un WCET probado. Para `k_sem`, los 51.604 ms
son el ancho de GPIO4 mientras low conserva el semáforo e incluyen la ejecución
de medium; no son una cota de bloqueo garantizada por el protocolo. La captura
con semáforo demuestra que la inversión permite una demora mayor. El mutex PI
reduce la respuesta medida, aunque esta sección crítica sigue siendo demasiado
larga para que el control cumpla 10 ms.

Una captura finita, incluso de 30 s, puede quedar debajo de una cota de RTA:
la cota contempla la interferencia y la fase de liberaciones más desfavorables,
mientras que la prueba solo observa las fases que ocurrieron durante su ventana.
Los máximos medidos no reemplazan una cota WCET.
