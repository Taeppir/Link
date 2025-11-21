from __future__ import annotations
from PySide6.QtWidgets import (
    QWidget, QVBoxLayout, QLabel, QTabWidget,
    QTableWidget, QHeaderView, QPushButton, QGroupBox)
from PySide6.QtCore import Qt
from .report_slots import ReportSlots

class ReportPanel(QGroupBox):
    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__("경로 탐색 결과", parent)

        outer = QVBoxLayout(self)
        outer.setSpacing(10)

        # --- 경로 탐색 전 안내 문구 ---
        self.placeholder = QLabel("경로를 탐색해주세요")
        self.placeholder.setAlignment(Qt.AlignCenter)
        self.placeholder.setStyleSheet("color: #666; font-size: 14px; padding: 35px;")
        outer.addWidget(self.placeholder)

        # --- 보기 탭 ---
        self.tabs = QTabWidget()
        self.tabs.setVisible(False) # 탐색 전 숨김
        outer.addWidget(self.tabs, 1)

        # --- 파일 다운로드 버튼 ---
        self.download_btn = QPushButton("[결과 다운로드]")
        self.download_btn.setToolTip("경로 탐색 결과를 .xlsx 파일로 다운로드합니다.")
        self.download_btn.setCursor(Qt.PointingHandCursor)
        self.download_btn.setFixedHeight(26)
        self.download_btn.setStyleSheet("""
            QPushButton {
                border: none;
                background: transparent;
                color: #2b6cb0;
                font-size: 12px;
                padding-right: 6px;
                padding-bottom: 10px;
            }
            QPushButton:hover {
                color: #1e4e8c;
                text-decoration: underline;
            }
        """)
        self.tabs.setCornerWidget(self.download_btn, Qt.TopRightCorner)

        # 보기 탭
        self.compare_tab = self.createReportTab("비교", ["최적", "최단"], has_detail=False, has_summary_text=True)
        self.optimal_tab = self.createReportTab("최적 경로", ["최적"], has_detail=True, has_summary_text=False)
        self.shortest_tab = self.createReportTab("최단 경로", ["최단"], has_detail=True, has_summary_text=False)

        self.tabs.addTab(self.compare_tab, "비교")
        self.tabs.addTab(self.optimal_tab, "최적 경로")
        self.tabs.addTab(self.shortest_tab, "최단 경로")

        # --- 슬롯 연결 ---
        self.slots = ReportSlots(self)
        self.download_btn.clicked.connect(self.slots.download_report)

    # --- 탭 생성 ---
    def createReportTab(self, title: str, row_labels: list[str],
                        has_detail: bool, has_summary_text: bool) -> QWidget:
        tab = QWidget()
        layout = QVBoxLayout(tab)
        layout.setSpacing(8)
        layout.setContentsMargins(4, 4, 4, 4)

        # 요약
        summary_label = QLabel("요약")
        layout.addWidget(summary_label)

        summary = QTableWidget(len(row_labels), 6, self)
        summary.setHorizontalHeaderLabels([
            "출발시간", "예상 도착시간", "총 항해거리(km)",
            "총 항해시간(h)", "예상 연료 소모량(ton)", "속력(m/s)"
        ])
        summary.setVerticalHeaderLabels(row_labels)
        summary.verticalHeader().setVisible(True)
        summary.horizontalHeader().setSectionResizeMode(QHeaderView.Stretch)
        summary.verticalHeader().setSectionResizeMode(QHeaderView.Stretch)
        summary.verticalHeader().setDefaultAlignment(Qt.AlignCenter)
        summary.setStyleSheet("QTableWidget { gridline-color: #c8c8c8; }")

        row_height = 35
        total_height = (row_height * len(row_labels)) + summary.horizontalHeader().height() + 6
        summary.setFixedHeight(total_height)
        layout.addWidget(summary)

        # 상세
        detail = None
        if has_detail:
            detail_label = QLabel("상세 보기")
            layout.addWidget(detail_label)

            detail = QTableWidget(8, 7, self)
            detail.setHorizontalHeaderLabels([
                "지도 위치 보기", "상세 보기", "위도", "경도", "예상 도착시간",
                "예상 연료 소모량(ton)", "속력"
            ])
            detail.horizontalHeader().setSectionResizeMode(QHeaderView.Stretch)
            detail.verticalHeader().setVisible(False)
            detail.setStyleSheet("QTableWidget { gridline-color: #c8c8c8; }")
            layout.addWidget(detail, 1)
        else:
            layout.addStretch(1)

        # 요약 문구
        if has_summary_text:
            summary_text = QLabel()
            summary_text.setWordWrap(True)
            summary_text.setAlignment(Qt.AlignLeft | Qt.AlignTop)
            summary_text.setTextFormat(Qt.RichText)
            summary_text.setStyleSheet("font-size: 12px; margin-top: 4px; color: #333;")
            layout.addWidget(summary_text)
            tab.summary_text = summary_text
        else:
            tab.summary_text = None

        tab.summary = summary
        tab.detail = detail
        return tab

    # -- Report 패널 활성화 --
    def showReport(self, visible: bool):
        self.tabs.setVisible(visible)
        self.placeholder.setVisible(not visible)
