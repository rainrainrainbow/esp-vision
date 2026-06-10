MicroPython Language Fundamentals
=================================

:link_to_translation:`zh_CN:[中文]`

MicroPython is a compact implementation of the Python language designed to run directly on microcontrollers. ESP-VISION uses MicroPython v1.28.0 as its application language while retaining C/C++ and ESP-IDF components for hardware access and performance-critical vision work. This chapter introduces the language model needed to read and write ESP-VISION applications; the authoritative details remain in the `MicroPython v1.28.0 language and implementation reference <https://docs.micropython.org/en/v1.28.0/reference/index.html>`_.

Execution Model
---------------

Upstream MicroPython aims to implement Python 3.4 syntax with selected features from later Python versions. ESP-VISION builds the ESP32 port at the ``EXTRA_FEATURES`` configuration level, with the on-device compiler, garbage collector, MPZ long integers, single-precision floating point, scheduler, VFS, and persistent bytecode loading enabled. Source code remains dynamically typed Python, but it is compiled to compact bytecode and executed by the MicroPython virtual machine. Native modules connect Python objects to ESP-VISION C/C++ components, ESP-IDF drivers, and chip hardware:

.. blockdiag::

   blockdiag {
     orientation = portrait;

     source   [label = "Python source\n.py"];
     frozen   [label = "Precompiled or frozen module\n.mpy / firmware"];
     bytecode [label = "MicroPython bytecode"];
     vm       [label = "MicroPython virtual machine"];
     modules  [label = "Python and native modules\nsensor / image / espdl / machine"];
     native   [label = "ESP-VISION C/C++ components\nand ESP-IDF drivers"];
     hardware [label = "Camera, display, storage,\nnetwork, and chip accelerators"];

     source -> bytecode;
     frozen -> bytecode;
     bytecode -> vm -> modules -> native -> hardware;
   }

Code loaded from ``.py`` is compiled on the device and consumes RAM during compilation. Precompiled ``.mpy`` files and modules frozen into the firmware reduce runtime compilation work; immutable constants in frozen modules can remain in Flash. Python still owns application policy and object lifetimes, while native components implement operations such as frame capture, image processing, codecs, and AI inference.

Basic Syntax
------------

The current ESP-VISION configuration uses indentation to define blocks and supports the familiar Python forms for assignment, expressions, conditions, loops, functions, classes, exceptions, context managers, comprehensions, generators, imports, and ``async``/``await``. Names are dynamically bound to objects, so a declaration does not specify a C-style storage type:

.. code-block:: python

   from micropython import const

   _MIN_PIXELS = const(80)

   class Detection:
       def __init__(self, label, score):
           self.label = label
           self.score = score

       def accepted(self, threshold=0.5):
           return self.score >= threshold

   def select_results(results):
       selected = []
       for label, score in results:
           item = Detection(label, score)
           if item.accepted():
               selected.append(item)
       return selected

   try:
       detections = select_results((("person", 0.91), ("chair", 0.42)))
       for item in detections:
           print(item.label, item.score)
   except (ValueError, TypeError) as error:
       print("invalid result:", error)
   finally:
       print("processing complete")

``const()`` is a MicroPython extension that lets the compiler substitute a constant value and can reduce bytecode and global-dictionary overhead. Type annotations may be useful to developers and editors, but MicroPython does not use them to provide C-style static typing or automatic memory layout.

Main Data Structures
--------------------

The choice between mutable and immutable objects matters on a microcontroller because creating, resizing, or concatenating objects allocates heap memory:

.. list-table::
   :header-rows: 1
   :widths: 20 18 62

   * - Type
     - Mutability
     - ESP-VISION usage
   * - ``None`` and ``bool``
     - Immutable
     - Represent absence and state flags without defining custom sentinel objects.
   * - ``int`` and ``float``
     - Immutable
     - Store counters, coordinates, thresholds, and scores. Repeated arithmetic can create new objects; avoid floating-point work in hard interrupt context.
   * - ``str``
     - Immutable
     - Store text, paths, labels, and JSON keys. Concatenation creates a new string, so avoid building large strings every frame.
   * - ``bytes``
     - Immutable
     - Store compact binary constants, encoded packets, and read-only model or protocol data.
   * - ``bytearray``
     - Mutable
     - Provide reusable binary buffers for peripheral I/O and packet assembly without allocating a new object for every operation.
   * - ``list``
     - Mutable
     - Store ordered results whose size changes. ``append()`` and growth may allocate memory, so bound or reuse lists in sustained frame loops.
   * - ``tuple``
     - Immutable
     - Represent fixed records such as rectangles and detection results. Constant tuples can be more memory-efficient than repeatedly created lists.
   * - ``dict``
     - Mutable
     - Represent configuration and shared scalar state. Adding keys can resize the table; create the expected structure during initialization.
   * - ``set``
     - Mutable
     - Perform membership and uniqueness checks when the additional hash-table memory is justified.
   * - ``range``
     - Immutable
     - Describe integer iteration without constructing a list of all values.
   * - ``memoryview`` and ``array``
     - View / mutable
     - Access typed or sliced buffer data with fewer copies; the referenced buffer must remain alive and must not be resized while a view is active.

Object references are shared rather than copied automatically. Assigning ``b = a`` makes both names refer to the same mutable list, dictionary, image, or buffer; use an explicit copy only when independent data is required. This is especially important for :py:class:`image.Image` objects backed by reusable camera frame buffers.

Design Principles for Microcontrollers
--------------------------------------

MicroPython prioritizes portability, interactive development, compact firmware integration, and direct hardware access over complete CPython library coverage. Its garbage-collected heap makes dynamic application code practical, but RAM, Flash, stack space, and allocation latency remain finite. The `MicroPython guide for constrained devices <https://docs.micropython.org/en/v1.28.0/reference/constrained.html>`_ recommends reducing repeated object creation, preallocating buffers, moving constants and reusable modules into frozen bytecode, and monitoring the heap with ``gc.mem_free()`` and ``gc.mem_alloc()``.

For ESP-VISION applications, initialize modules, models, buffers, and stable dictionaries before entering the frame loop. Reuse ``bytearray`` and image buffers, keep per-frame temporary data bounded, and avoid retaining a camera-backed image after the next ``sensor.snapshot()`` unless it has been copied. Use Python to express orchestration and product behavior; leave deterministic real-time work, ISR handling, bulk pixel processing, codecs, and inference kernels in native components.

MicroPython and CPython
-----------------------

In this documentation, “Python” in a runtime comparison means CPython 3.x, the reference implementation commonly used on PCs and servers. The exact MicroPython feature set is firmware-dependent, so this table describes MicroPython v1.28.0 as configured by ESP-VISION: all maintained board manifests freeze ``asyncio``, the ESP32 port enables ``_thread`` with a GIL, and hardware APIs remain subject to chip and board configuration.

.. list-table::
   :header-rows: 1
   :widths: 22 39 39

   * - Area
     - ESP-VISION MicroPython
     - CPython
   * - Primary environment
     - Runs inside MCU firmware with direct access to peripherals and board services.
     - Runs as an operating-system process on PCs, servers, and larger embedded systems.
   * - Language syntax
     - Implements Python 3.4 syntax with selected later features; most common control flow, functions, classes, exceptions, generators, and asynchronous syntax are available.
     - Tracks the current Python language specification and generally introduces new syntax first.
   * - Type system
     - Dynamically typed; annotations do not make execution statically typed.
     - Dynamically typed; annotations are consumed mainly by tools and optional libraries.
   * - Standard library
     - Provides a resource-oriented subset selected at firmware build time, plus ``machine``, ``micropython``, ``esp``, and ``esp32`` hardware/runtime modules.
     - Provides the full CPython standard library for the installed version, without a standard MCU peripheral API.
   * - Package deployment
     - Modules are copied to storage, precompiled as ``.mpy``, or frozen into firmware; desktop ``pip`` and arbitrary binary wheels must not be assumed.
     - Commonly uses ``pip``, virtual environments, source distributions, and platform wheels.
   * - Memory model
     - Uses a small garbage-collected heap shared with firmware resources; allocation failures and fragmentation must be considered explicitly.
     - Uses a much larger process heap and reference counting with cyclic garbage collection.
   * - Concurrency
     - Provides frozen ``asyncio``, enabled ``_thread`` with a GIL, and scheduled GPIO IRQ and Timer callbacks; peripheral availability still depends on the selected chip and board.
     - Provides ``asyncio``, threads, processes, and extensive operating-system synchronization facilities.
   * - Performance path
     - Calls native C/C++ modules and hardware accelerators for demanding operations; Python is best used for orchestration.
     - Can use optimized extension modules, JIT-enabled alternative runtimes, or multiprocessing, but usually has greater CPU and memory resources.
   * - Compatibility details
     - Some built-in behavior, exception text, introspection, module contents, and edge cases differ to reduce code size and RAM usage.
     - Defines the reference behavior against which MicroPython differences are documented.
   * - Portability
     - Pure Python code using the supported subset is often portable; hardware modules and resource assumptions are device-specific.
     - Pure Python code is portable across supported operating systems when its dependencies are available.

Do not decide portability from syntax alone. Check imported modules, memory use, filesystem paths, concurrency assumptions, and hardware access. The upstream `MicroPython and CPython differences <https://docs.micropython.org/en/v1.28.0/genrst/index.html>`_ list documents known behavioral differences, while :doc:`../api-reference/micropython` describes the inherited APIs and concurrency model available in ESP-VISION.
