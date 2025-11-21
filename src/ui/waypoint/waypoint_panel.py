from __future__ import annotations
from typing import TYPE_CHECKING
from ..constants import START, VIA, END, WAYPOINT_TYPES

from PySide6.QtCore import Qt, Signal, QDateTime, QSize, Slot
from PySide6.QtWidgets import (
    QGroupBox, QVBoxLayout, QLabel, QTableWidget, QHeaderView,
    QPushButton, QWidget, QDateTimeEdit, QHBoxLayout, QComboBox, QLineEdit,
    QStyle, QSizePolicy, QFrame, QTableWidgetItem
)
from PySide6.QtGui import QDoubleValidator

from .waypoint_slots import WaypointSlots

if TYPE_CHECKING:
    from ..map.map_panel import MapPanel


class WaypointPanel(QGroupBox):
    routeRequested = Signal(dict) # 탐색 결과 요청
    waypointsChanged = Signal(dict) # 지도 즉시 반영

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__("출도착지/경유지 등록", parent)
        v = QVBoxLayout(self)
        v.setSpacing(10)

        # --- 출발 시각 ---
        depart_time_layout = QHBoxLayout()
        depart_time_layout.setAlignment(Qt.AlignLeft)

        label_depart = QLabel("출발 시각:")
        self.depart_time = QDateTimeEdit(self)
        self.depart_time.setDisplayFormat("yyyy-MM-dd HH:mm:ss")
        self.depart_time.setCalendarPopup(True)
        self.depart_time.setDateTime(QDateTime.currentDateTime())
        self.depart_time.setFixedWidth(180)

        depart_time_layout.addWidget(label_depart)
        depart_time_layout.addWidget(self.depart_time)
        depart_time_layout.addStretch()
        v.addLayout(depart_time_layout)

        # --- 경유지 입력 폼  ---
        input_form_layout = QHBoxLayout()
        self.type = QComboBox()
        self.type.addItems(WAYPOINT_TYPES)
        self.type.setFixedWidth(70)

        self.port = QLineEdit()
        self.btn_search_port = QPushButton("검색")
        self.btn_search_port.setFixedWidth(50)
        self.lat = QLineEdit()
        self.lon = QLineEdit()

        # 위/경도 숫자 전용 입력 제한
        lat_validator = QDoubleValidator(-90.0, 90.0, 9)
        lon_validator = QDoubleValidator(-180.0, 180.0, 9)
        lat_validator.setNotation(QDoubleValidator.StandardNotation)
        lon_validator.setNotation(QDoubleValidator.StandardNotation)
        self.lat.setValidator(lat_validator)
        self.lon.setValidator(lon_validator)

        # placeholder 설정
        self.lat.setPlaceholderText("예: 35.1796")
        self.lon.setPlaceholderText("예: 129.0756")

        # 추가 버튼
        self.btn_add = QPushButton("추가")
        self.btn_add.setMinimumHeight(26)
        self.btn_add.setToolTip("경유지 추가")

        # 입력 폼 구성
        input_form_layout.addWidget(QLabel("유형:"))
        input_form_layout.addWidget(self.type, 1)
        input_form_layout.addWidget(QLabel("항구:"))
        input_form_layout.addWidget(self.port, 3)
        input_form_layout.addWidget(self.btn_search_port)
        input_form_layout.addWidget(QLabel("위도:"))
        input_form_layout.addWidget(self.lat, 2)
        input_form_layout.addWidget(QLabel("경도:"))
        input_form_layout.addWidget(self.lon, 2)
        input_form_layout.addWidget(self.btn_add, 1)
        v.addLayout(input_form_layout)

        # --- 구분선 ---
        line = QFrame()
        line.setFrameShape(QFrame.HLine)
        line.setStyleSheet("color: #e0e0e0")
        v.addWidget(line)

        # --- 테이블 + 버튼 그룹 ---
        table_with_buttons = QHBoxLayout()

        # 테이블
        self.table = QTableWidget(0, 4, self)
        self.table.setHorizontalHeaderLabels(["유형", "항구", "위도", "경도"])
        self.table.horizontalHeader().setSectionResizeMode(QHeaderView.Stretch)
        self.table.verticalHeader().setVisible(True)
        self.table.setShowGrid(True)
        self.table.setAlternatingRowColors(True)
        self.table.setEditTriggers(QTableWidget.NoEditTriggers)
        self.table.setSelectionBehavior(QTableWidget.SelectRows)

        header = self.table.horizontalHeader()
        header.setStretchLastSection(False)
        header.setSectionResizeMode(0, QHeaderView.Fixed)
        header.setSectionResizeMode(1, QHeaderView.Fixed)
        header.setSectionResizeMode(2, QHeaderView.Stretch)
        header.setSectionResizeMode(3, QHeaderView.Stretch)
        header.resizeSection(0, 80)
        header.resizeSection(1, 300)

        table_with_buttons.addWidget(self.table, 1)

        # 버튼 그룹(삭제,초기화)
        btn_panel = QVBoxLayout()
        btn_panel.setAlignment(Qt.AlignTop)

        self.btn_delete = QPushButton()
        self.btn_delete.setIcon(self.style().standardIcon(QStyle.SP_TrashIcon))
        self.btn_delete.setIconSize(QSize(18, 18))
        self.btn_delete.setToolTip("선택된 행 삭제")
        self.btn_delete.setFixedSize(50, 32)
        btn_panel.addWidget(self.btn_delete)

        self.btn_clear = QPushButton()
        self.btn_clear.setIcon(self.style().standardIcon(QStyle.SP_BrowserReload))
        self.btn_clear.setIconSize(QSize(18, 18))
        self.btn_clear.setToolTip("테이블 초기화")
        self.btn_clear.setFixedSize(50, 32)
        btn_panel.addWidget(self.btn_clear)

        btn_panel.addStretch()
        table_with_buttons.addLayout(btn_panel)
        v.addLayout(table_with_buttons)

        # --- 경로 탐색 버튼 (가로 전체 폭) ---
        self.btn_search = QPushButton("경로 탐색", self)
        self.btn_search.setMinimumHeight(45)
        self.btn_search.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        v.addWidget(self.btn_search)

        # --- 슬롯 연결 ---
        self.slots = WaypointSlots(self)
        self.btn_add.clicked.connect(self.slots.add_row)
        self.btn_delete.clicked.connect(self.slots.delete_selected) 
        self.btn_clear.clicked.connect(self._on_clear_clicked)
        self.btn_search.clicked.connect(self.slots.emit_route)
        self.btn_search_port.clicked.connect(self.slots.open_port_search)
        self._map_panel: "MapPanel | None" = None

    def attach_map_panel(self, map_panel: "MapPanel") -> None:
        """
        MapPanel과의 연결을 설정한다.
        - 웹(Leaflet)에서 우클릭 추가된 포인트를 오른쪽 테이블에 즉시 반영
        - 초기화 버튼 클릭 시 맵의 인터랙션도 함께 초기화
        """
        self._map_panel = map_panel
        map_panel.interactivePointAdded.connect(self.add_point_from_web)

    @Slot()
    def _on_clear_clicked(self) -> None:
        did_clear = self.slots.clear_table()
        if did_clear and self._map_panel is not None and hasattr(self._map_panel, "clear_interactive"):
            self._map_panel.clear_interactive()
    @Slot(str, float, float, str)
    def add_point_from_web(self, t: str, lat: float, lon: float, port: str = "") -> None:
        table = self.table
        row_count = table.rowCount()
        if row_count == 0:
            type_text = START
        else:
            type_text = t if t in WAYPOINT_TYPES else VIA

        if type_text == START:
            row = 0
        elif type_text == END:
            row = row_count
        else:
            row = row_count
            if row_count > 0:
                last_cell = table.item(row_count - 1, 0)
                if last_cell and last_cell.text() == END:
                    row -= 1

        lat_str = "{:.6f}".format(float(lat)) if lat is not None else ""
        lon_str = "{:.6f}".format(float(lon)) if lon is not None else ""

        table.blockSignals(True)
        try:
            table.insertRow(row)
            table.setItem(row, 0, QTableWidgetItem(type_text))
            table.setItem(row, 1, QTableWidgetItem(port or ""))
            table.setItem(row, 2, QTableWidgetItem(lat_str))
            table.setItem(row, 3, QTableWidgetItem(lon_str))

            for c in (2, 3):
                it = table.item(row, c)
                if it:
                    it.setTextAlignment(Qt.AlignRight | Qt.AlignVCenter)

            table.selectRow(row)
        finally:
            table.blockSignals(False)

        table.viewport().update()