# SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
#
# SPDX-License-Identifier: Apache-2.0

from __future__ import annotations

from typing import Sequence

from image import Image

ROI = tuple[int, int, int, int] | Sequence[int]


class ESP32Display:
    def __init__(self, width: int = 0, height: int = 0, refresh: int = 60, *, backlight: int = 100) -> None: ...
    def __del__(self) -> None: ...
    def deinit(self) -> None: ...
    def width(self) -> int: ...
    def height(self) -> int: ...
    def clear(self, display_off: bool = False) -> None: ...
    def backlight(self, value: int | None = None) -> int | None: ...
    def write(
        self,
        image: Image,
        *,
        x: int = 0,
        y: int = 0,
        x_scale: float | None = None,
        y_scale: float | None = None,
        roi: ROI | None = None,
        fit: bool = True,
    ) -> None: ...


Display = ESP32Display
