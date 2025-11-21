import csv
from PySide6.QtWidgets import (
    QDialog, QVBoxLayout, QLineEdit, QTableWidget, QTableWidgetItem,
    QPushButton, QHBoxLayout, QAbstractItemView, QHeaderView
)
from PySide6.QtCore import Qt

class PortSearchDialog(QDialog):
    def __init__(self, csv_path, parent=None):
        super().__init__(parent)
        self.setWindowTitle("항구 검색")
        self.resize(511, 600)

        # 선택한 항구 변수
        self.selected_port = None

        v = QVBoxLayout(self)

        # 검색창
        self.search_box = QLineEdit()
        self.search_box.setPlaceholderText("항구명 또는 인덱스를 입력하세요")
        self.search_box.textChanged.connect(self.filter_ports)

        v.addWidget(self.search_box)

        # 항구 리스트 테이블
        self.table = QTableWidget(0, 4)
        self.table.setHorizontalHeaderLabels(["Port Index","항구명", "위도", "경도"])
        self.table.setSelectionBehavior(QAbstractItemView.SelectRows)
        self.table.verticalHeader().setVisible(False)
        self.table.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOn)
        self.table.setSelectionMode(QAbstractItemView.SingleSelection)

        header = self.table.horizontalHeader()
        header.setSectionResizeMode(QHeaderView.Fixed)
        header.resizeSection(0, 70)
        header.resizeSection(1, 200)
        header.resizeSection(2, 100)
        header.resizeSection(3, 100)

        self.table.cellDoubleClicked.connect(self.select_port)
        v.addWidget(self.table)

        # 선택 버튼
        h = QHBoxLayout()
        self.btn_ok = QPushButton("선택")
        self.btn_ok.clicked.connect(self.confirm)
        h.addStretch()
        h.addWidget(self.btn_ok)
        v.addLayout(h)

        # CSV 로드
        self.port_data = []
        self.load_csv(csv_path)
        self.show_ports(self.port_data)

    # CSV 로드
    def load_csv(self, path):
        try:
            with open(path, "r", encoding="utf-8") as f:
                reader = csv.DictReader(f)
                for row in reader:
                    self.port_data.append({
                        "index": row["World Port Index Number"],
                        "name": row["Main Port Name"],
                        "lat": row["Latitude"],
                        "lon": row["Longitude"]
                    })

        except FileNotFoundError:
            print(f"[PortSearchDialog][ERROR] CSV 파일을 찾을 수 없습니다: {path}")

        except Exception as e:
            print(f"[PortSearchDialog][ERROR] 알 수 없는 오류 발생: {e}")

    # 테이블에 표시
    def show_ports(self, ports):
        self.table.setRowCount(len(ports))
        for i, p in enumerate(ports):
            self.table.setItem(i, 0, QTableWidgetItem(str(p["index"])))
            self.table.setItem(i, 1, QTableWidgetItem(p["name"]))
            self.table.setItem(i, 2, QTableWidgetItem(str(p["lat"])))
            self.table.setItem(i, 3, QTableWidgetItem(str(p["lon"])))

    # 검색 필터
    def filter_ports(self):
        keyword = self.search_box.text().lower()

        filtered = []

        # -- 항구 이름 / 인덱스 검색 --
        for p in self.port_data:
            name_match = keyword in p["name"].lower()
            index_match = keyword in p["index"].lower()

            if name_match or index_match:
                filtered.append(p)

        self.show_ports(filtered)

    # 더블 클릭 시 선택 처리
    def select_port(self, row, col):
        self.selected_port = {
            "name": self.table.item(row, 1).text(),
            "lat": self.table.item(row, 2).text(),
            "lon": self.table.item(row, 3).text()
        }
        self.accept()

    # 선택 버튼 클릭 시 선택 처리
    def confirm(self):
        row = self.table.currentRow()
        if row >= 0:
            self.select_port(row, 0)