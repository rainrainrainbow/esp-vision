# ESP-VISION VSCode Extension

This extension provides a minimal MicroPython raw REPL workflow for ESP-VISION
boards.

Current commands:

- `ESP-VISION: Connect`
- `ESP-VISION: Disconnect`
- `ESP-VISION: Run Current File`
- `ESP-VISION: Stop Script`
- `ESP-VISION: Soft Reset`

The same actions are also exposed as status bar buttons.

The first version communicates over a serial CDC port and uses the standard
MicroPython raw REPL protocol. Frame preview support will be added later on top
of a dedicated ESP-VISION protocol.

Default serial baud rate is `921600`, matching the OpenMV workflow. It can be
overridden with the `espVision.baudRate` setting.
