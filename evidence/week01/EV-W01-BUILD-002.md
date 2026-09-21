# EV-W01-BUILD-002 — NUCLEO-G474RE `blinky` build

## Identification

| Field | Value |
|---|---|
| Date | 2026-09-20 |
| Operator | `ing_juanvrg` |
| Objective | Generate the Zephyr `blinky` firmware for the physical starter board |
| Platform | NUCLEO-G474RE (`nucleo_g474re/stm32g474xx`) |
| Verdict | PASS — build only |

## Reproduction

```bash
source ~/zephyrproject/.venv/bin/activate
cd ~/zephyrproject/zephyr

west build \
  -p always \
  -b nucleo_g474re \
  samples/basic/blinky \
  -d build/blinky-nucleo-g474re
```

## Observed result

- Build completed: 159/159 steps.
- Output linked: `build/blinky-nucleo-g474re/zephyr/zephyr.elf`.
- Zephyr revision: `v4.4.0-13939-g2f0bc11264a8`.

| Memory region | Used | Available | Reported use |
|---|---:|---:|---:|
| Flash | 18,896 B | 512 KB | 3.60% |
| RAM | 4,544 B | 128 KB | 3.47% |

## Interpretation and limitation

Zephyr recognized the board definition, selected the ARM target, processed the
configuration and devicetree, and produced STM32G474 firmware. This evidence does
not prove that ST-LINK flashing, reset, GPIO output, or the physical LD2 LED works;
those require a separate run-on-hardware record.
