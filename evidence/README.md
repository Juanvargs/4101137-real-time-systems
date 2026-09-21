# Timing evidence archive

This directory stores the records and artifacts cited by `RET.md`.

## Naming

Use `EV-WNN-TYPE-NNN-short-description`:

- `WNN`: course week (`W01`, `W02`, ...).
- `TYPE`: `ENV`, `GPIO`, `TRACE`, `RTA`, `AB`, `LINUX`, `SAFE`, or another short
  stable category.
- `NNN`: sequence number within the week/category.

Example: `EV-W04-AB-001-superloop-vs-kernel`.

## Minimum record

Every evidence record must include:

- objective and linked requirement IDs;
- platform and board identifier;
- firmware commit and build configuration;
- exact workload and observation interval;
- instrument and relevant resolution/settings;
- commands required to reproduce the result;
- raw-data and processed-figure paths;
- result, deadline margin, and verdict.

Do not edit raw captures. If data must be cleaned or transformed, preserve the
original and record the script or command that produced the derived file.
