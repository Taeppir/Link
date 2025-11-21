from __future__ import annotations
from PySide6.QtCore import Signal
from PySide6.QtWidgets import QWidget, QVBoxLayout, QPushButton


class SideTools(QWidget):
    depthToggled = Signal(bool)
    coastlineToggled = Signal(bool)
    weatherToggled = Signal(bool)

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        v = QVBoxLayout(self)
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(8)

        self.btn_depth = QPushButton("수심 지도")
        self.btn_depth.setCheckable(True)
        self.btn_depth.toggled.connect(self.depthToggled)

        self.btn_coast = QPushButton("국가 경계선 지도")
        self.btn_coast.setCheckable(True)
        self.btn_coast.toggled.connect(self.coastlineToggled)

        self.btn_weather = QPushButton("날씨")
        self.btn_weather.setCheckable(True)
        self.btn_weather.toggled.connect(self.weatherToggled)

        for b in (self.btn_depth, self.btn_coast, self.btn_weather):
            b.setMinimumHeight(44)
            v.addWidget(b)

        v.addStretch(1)
        self.setFixedWidth(120)
