# SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
#
# SPDX-License-Identifier: Apache-2.0

from __future__ import annotations

from typing import Sequence

from image import Image

Float3 = tuple[float, float, float] | Sequence[float]
Detection = tuple[int, int, int, int, float, int]
Classification = tuple[str, float]


def load_model(path: str, *, profile: bool = False) -> bool: ...


class ESPDet:
    def __init__(
        self,
        path: str,
        *,
        score: float | None = None,
        nms: float | None = None,
        mean: Float3 | None = None,
        std: Float3 | None = None,
    ) -> None: ...
    def __del__(self) -> None: ...
    def deinit(self) -> None: ...
    def detect(self, image: Image) -> list[Detection]: ...
    def set_thresholds(self, *, score: float | None = None, nms: float | None = None) -> ESPDet: ...


class YOLO11:
    def __init__(
        self,
        path: str,
        *,
        score: float | None = None,
        nms: float | None = None,
        topk: int = 10,
        mean: Float3 | None = None,
        std: Float3 | None = None,
    ) -> None: ...
    def __del__(self) -> None: ...
    def deinit(self) -> None: ...
    def detect(self, image: Image) -> list[Detection]: ...
    def set_thresholds(self, *, score: float | None = None, nms: float | None = None) -> YOLO11: ...


class ImageNetCls:
    def __init__(
        self,
        path: str,
        *,
        topk: int = 5,
        score: float | None = None,
        mean: Float3 | None = None,
        std: Float3 | None = None,
        softmax: bool = True,
    ) -> None: ...
    def __del__(self) -> None: ...
    def deinit(self) -> None: ...
    def classify(self, image: Image) -> list[Classification]: ...
    def set_thresholds(self, *, topk: int | None = None, score: float | None = None) -> ImageNetCls: ...
