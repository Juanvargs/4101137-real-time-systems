# EV-W01-ENV-001 — Zephyr `native_sim` prerequisite

## Identification

| Field | Value |
|---|---|
| Date | 2026-09-20 |
| Operator | `ing_juanvrg` |
| Objective | Verify the mandatory no-hardware Zephyr build and execution path |
| Related requirement | Course prerequisite; no product timing requirement |
| Verdict | PASS |

## Environment

| Item | Value |
|---|---|
| Host OS | Linux |
| `west` | 1.5.0 |
| Zephyr source | 4.4.99 |
| Zephyr build revision | `v4.4.0-13939-g2f0bc11264a8` |
| Board target | `native_sim/native` |
| Physical board confirmed | NUCLEO-G474RE; Zephyr target `nucleo_g474re` |
| Build directory | `~/zephyrproject/zephyr/build/hello_world-native` |
| Course-repository baseline | `cdc0471` |
| ARM toolchain | `arm-zephyr-eabi` installed |
| ESP32-S3 toolchain | `xtensa-espressif_esp32s3_zephyr-elf` installed |
| Serial-access group | `dialout` present |
| VS Code CLI | Not found; editor selection remains open |
| pyOCD | 0.45.1 |
| Debug probe | STLINK-V3; accessible as an unprivileged user |
| pyOCD target | `stm32g474retx` from `Keil.STM32G4xx_DFP` 2.2.0 |

## Reproduction

```bash
source ~/zephyrproject/.venv/bin/activate
cd ~/zephyrproject/zephyr

west build \
  -p always \
  -b native_sim \
  samples/hello_world \
  -d build/hello_world-native

./build/hello_world-native/zephyr/zephyr.exe
```

## Troubleshooting record

The first student-run attempt used the shared default `build/` directory. CMake
rejected it because its cache had been generated from
`~/zephyrproject/zephyr/CMakeLists.txt`, while the requested application source was
`samples/hello_world/CMakeLists.txt`. Since configuration failed, no `zephyr.exe`
was produced.

The successful attempt used an application-specific build directory and an
explicit pristine policy: `-d build/hello_world-native -p always`. This prevents
CMake cache state from one application from being reused for another.

## Observed result

The student-run build completed all 99 build steps successfully. Execution
produced:

```text
*** Booting Zephyr OS build v4.4.0-13939-g2f0bc11264a8 ***
Hello World! native_sim/native
```

## Interpretation

The Zephyr workspace, Python environment, host toolchain, devicetree generation,
Kconfig processing, compilation, linking, and `native_sim` execution path are
operational. This result does not verify physical-board flashing, serial access, or
any real-time property of the SoilSense firmware.

The required ARM and ESP32-S3 cross-toolchains are installed and the user belongs
to `dialout`. Physical serial access remains unverified until a course board is
connected. VS Code was not found through the `code` command; this is not a course
blocker if another editor is explicitly selected and can open the workspace.

## USB permission recovery

The STLINK-V3 was visible to `lsusb` but initially unavailable to pyOCD as an
unprivileged user. A device-specific `udev` rule assigns the probe to `plugdev`
with mode `0660`:

```text
ATTR{idVendor}=="0483",ATTR{idProduct}=="374e",GROUP="plugdev",MODE="0660"
```

During setup, this rule was accidentally split across two physical lines. The
unconditional second line changed `/dev/null` to mode `0660`, preventing normal
users from writing to it. The rule was corrected to one line and the host was
rebooted. Final checks showed `/dev/null` at `0666` and pyOCD detecting the
NUCLEO-G474RE without `sudo`, with target support installed.
