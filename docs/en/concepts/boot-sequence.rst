Startup Sequence
================

:link_to_translation:`zh_CN:[中文]`

ESP-VISION follows the MicroPython reset and boot model while adding initialization for the Flash filesystem, board storage, camera, display, and preview services. This chapter describes the sequence implemented by the current firmware. For the upstream model and general behavior, see the `MicroPython v1.28.0 Reset and Boot Sequence <https://docs.micropython.org/en/v1.28.0/reference/reset_boot.html>`_.

Hard Reset and Soft Reset
-------------------------

A hard reset restarts the MCU and ESP-IDF runtime before creating a new MicroPython environment. It occurs after power-on, the board reset button, ``machine.reset()``, watchdog or brownout reset, and wake-up from deep sleep. Use ``machine.reset_cause()`` when an application needs to distinguish the reset source.

A soft reset restarts the MicroPython environment without restarting the complete MCU runtime. It can be requested with ``Ctrl-D`` in the friendly REPL or ``machine.soft_reset()``. ESP-VISION clears Python objects and modules, closes files and sockets through MicroPython cleanup, releases camera, display, preview, USB, PWM, timer, UART, thread, and other managed resources, and then repeats the Python startup sequence.

Some system state can survive a soft reset, including the RTC, CPU clock configuration, and an active network interface at the IP layer. Application code must not assume that Python objects representing those resources survive; recreate the objects and verify their state after every reset.

ESP-VISION Startup Order
------------------------

After a hard or soft reset, ESP-VISION executes the following startup flow:

.. blockdiag::

   blockdiag {
     orientation = portrait;

     reset    [label = "Hard or soft reset"];
     runtime  [label = "Initialize MicroPython runtime\nand machine peripherals"];
     services [label = "Initialize camera, display,\npreview, storage, and framebuffer"];
     boot     [label = "Run frozen _boot.py\nmount internal Flash at /"];
     setup    [label = "Run frozen py_inisetup.py\ninitialize or repair filesystem"];
     sdcard   [label = "Mount SD card at /sdcard\nwhen available"];
     bootpy   [label = "Run /boot.py\nwhen present"];
     usb      [label = "Initialize USB device"];
     replmode [label = "REPL mode", shape = diamond];
     mainpy   [label = "Run /main.py\nwhen present"];
     repl     [label = "Enter friendly or raw REPL"];

     reset -> runtime -> services -> boot -> setup -> sdcard -> bootpy -> usb -> replmode;
     replmode -> mainpy [label = "friendly"];
     replmode -> repl [label = "raw"];
     mainpy -> repl [label = "exit, interrupt, or skip"];
   }

The raw REPL used by host automation can skip ``main.py`` during a soft reset. This allows development tools to gain control without automatically starting the product application.

First Boot and Flash Filesystem
-------------------------------

The frozen ``_boot.py`` only attempts to mount the internal Flash filesystem. ESP-VISION-specific setup is handled by ``py_inisetup.py``. If the Flash boot sector is empty, it formats the configured ``vfs`` or ``ffat`` partition and mounts it at ``/``. It then creates ``/boot.py``, ``/main.py``, ``/README.txt``, and the ``/.esp_vision_disk`` marker when they do not already exist.

If mounting or writing the default files fails, ``py_inisetup.py`` attempts to repair the filesystem by formatting it and recreating the defaults. Formatting erases files stored in that filesystem, so product data that must survive filesystem recovery should be stored in a separate partition, on an SD card, or outside the device.

Existing startup files are not overwritten during a normal firmware update or soft reset. Board packages can provide board-specific default ``main.py`` and ``README.txt`` content through ``boards/<BOARD>/board_inisetup.py``.

Using boot.py
-------------

Use ``boot.py`` for short, deterministic initialization that must complete before the application starts, such as selecting a product mode, preparing configuration, or bringing up a required network interface:

.. code-block:: python

   import network

   wlan = network.WLAN(network.STA_IF)
   wlan.active(True)

``boot.py`` must return and must not contain the application's permanent loop. ESP-VISION initializes the MicroPython USB device only after ``boot.py`` completes, so a blocked or long-running ``boot.py`` can prevent the USB REPL and host tools from becoming available.

Using main.py
-------------

Use ``main.py`` as the product application entry point. Keep the implementation in a separate module so startup policy and application logic remain independent:

.. code-block:: python

   import sys
   import my_app

   try:
       my_app.main()
   except KeyboardInterrupt:
       raise
   except Exception as error:
       print("Fatal application error:")
       sys.print_exception(error)

Allowing ``KeyboardInterrupt`` to propagate lets ``Ctrl-C`` stop the application and enter the friendly REPL. A production application can instead log the exception and call ``machine.reset()`` when automatic recovery is required, but an unconditional reset loop can make development and failure diagnosis difficult.

The default ESP-VISION ``main.py`` prints a board-ready message and sleeps in a loop so the VSCode extension and other host tools can take control. Press ``Ctrl-C`` to interrupt it and reach the REPL, or replace ``/main.py`` with the product entry point.

REPL and Recovery
-----------------

``main.py`` exiting normally or raising an uncaught exception transfers control to the friendly REPL. ``Ctrl-C`` injects ``KeyboardInterrupt`` into a running Python script, while ``Ctrl-D`` at the friendly REPL starts a soft reset.

If an application prevents normal startup, connect to the REPL, interrupt it with ``Ctrl-C``, and rename or remove the startup file:

.. code-block:: python

   import os

   print(os.listdir("/"))
   os.rename("/main.py", "/main.disabled.py")
   # Use Ctrl-D to start again without running the previous main.py.

If the REPL cannot be recovered, erase and reflash the board from the repository root. ``erase-flash`` removes the entire device Flash, including the internal filesystem and user data:

.. code-block:: bash

   idf.py --board <BOARD> -p <PORT> erase-flash
   idf.py --board <BOARD> -p <PORT> flash monitor

Reflashing without ``erase-flash`` normally preserves the data filesystem and therefore may preserve a faulty ``boot.py`` or ``main.py``.
