# EV-W03-PORT-001 — Port del superloop a ESP32-C6 SuperMini

## Objetivo

Verificar el port de `firmware/superloop` a la ESP32-C6 SuperMini mediante un
overlay Devicetree, sin modificar la lógica de la aplicación en C.

## Plataforma

- Placa: ESP32-C6 SuperMini
- Target Zephyr: `weact_esp32c6_mini/esp32c6/hpcore`
- Consola: USB Serial/JTAG, `/dev/ttyACM0`, 115200 8N1
- Aplicación: `firmware/superloop`

## Cambio realizado

Se añadió el archivo:

```text
firmware/superloop/boards/weact_esp32c6_mini_esp32c6_hpcore.overlay
