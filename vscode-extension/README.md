# ESP-VISION VSCode Extension

This extension provides a MicroPython raw REPL workflow for ESP-VISION boards.

Current commands:

- `ESP-VISION: Connect`
- `ESP-VISION: Disconnect`
- `ESP-VISION: Run Current File`
- `ESP-VISION: Stop Script`
- `ESP-VISION: Soft Reset`
- `ESP-VISION: Show Preview`

The same actions are also exposed as status bar buttons.

The extension communicates over a serial CDC port and uses the standard
MicroPython raw REPL protocol for script control.

Frame preview uses the ESP-VISION EVFRAME protocol. Scripts refresh the preview
by calling `img.flush()`.

The preview panel displays the latest JPG frame, frame rate, frame metadata, and
separate RGB channel histograms with per-channel min, max, and mean values.

Default serial baud rate is `921600`, matching the OpenMV workflow. It can be
overridden with the `espVision.baudRate` setting.
