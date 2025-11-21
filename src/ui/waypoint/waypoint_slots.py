from __future__ import annotations

from PySide6.QtCore import QObject
from PySide6.QtWidgets import QTableWidgetItem, QMessageBox

from ..constants import START, VIA, END, WAYPOINT_TYPES


class WaypointSlots(QObject):
    def __init__(self, panel):
        super().__init__()
        self.panel = panel

    # ----------------------------
    # 내부 공통: 테이블 -> payload 구성
    # ----------------------------
    def _build_payload(self) -> dict | None:
        table = self.panel.table
        row_count = table.rowCount()

        # 출발 시각
        dt = self.panel.depart_time.dateTime()
        depart_time = dt.toString("yyyy-MM-dd HH:mm:ss") if dt.isValid() else ""

        # 비어 있으면 빈 payload (지도 초기화용)
        if row_count == 0:
            return {"depart_time": depart_time, "waypoints": []}

        waypoints: list[dict] = []
        for r in range(row_count):
            it_type = table.item(r, 0)
            it_port = table.item(r, 1)
            it_lat  = table.item(r, 2)
            it_lon  = table.item(r, 3)

            t = it_type.text() if it_type else ""
            p = it_port.text() if it_port else ""
            lat_text = (it_lat.text().strip() if it_lat and it_lat.text() else "")
            lon_text = (it_lon.text().strip() if it_lon and it_lon.text() else "")

            # 위/경도는 지도에 찍기 전 숫자 변환 필요
            try:
                la = float(lat_text)
                lo = float(lon_text)
            except ValueError:
                # 좌표가 비어있거나 형식이 틀리면 지도 업데이트는 건너뜀
                return None

            waypoints.append({"type": t, "port": p, "lat": la, "lon": lo})

        return {"depart_time": depart_time, "waypoints": waypoints}

    # 행 추가
    def add_row(self):
        # --- 경고창 ---
        def validate(message):
            QMessageBox.warning(self.panel, "안내", message)
            return

        # -- 위도/경도 검증 ---
        lat_text = self.panel.lat.text().strip()
        lon_text = self.panel.lon.text().strip()

        # 1) 빈 입력 검증
        missing = []
        if not lat_text:
            self.panel.lat.setStyleSheet("border: 1px solid red;")
            missing.append("위도")
        else:
            self.panel.lat.setStyleSheet("")

        if not lon_text:
            self.panel.lon.setStyleSheet("border: 1px solid red;")
            missing.append("경도")
        else:
            self.panel.lon.setStyleSheet("")

        if missing:
            return validate(f"{', '.join(missing)} 입력이 비어 있습니다.")

        # 2) 형식 검증
        try:
            lat = float(lat_text)
            self.panel.lat.setStyleSheet("")
        except ValueError:
            self.panel.lat.setStyleSheet("border: 1px solid red;")
            return validate("위도는 숫자여야 합니다.")

        try:
            lon = float(lon_text)
            self.panel.lon.setStyleSheet("")
        except ValueError:
            self.panel.lon.setStyleSheet("border: 1px solid red;")
            return validate("경도는 숫자여야 합니다.")

        # 3) 범위 검증
        invalid_fields = []
        if not (-90.0 <= lat <= 90.0):
            self.panel.lat.setStyleSheet("border: 1px solid red;")
            invalid_fields.append("위도")
        else:
            self.panel.lat.setStyleSheet("")

        if not (-180.0 <= lon <= 180.0):
            self.panel.lon.setStyleSheet("border: 1px solid red;")
            invalid_fields.append("경도")
        else:
            self.panel.lon.setStyleSheet("")

        if invalid_fields:
            return validate(f"{', '.join(invalid_fields)}는 유효 범위를 벗어났습니다. (위도: -90~90, 경도: -180~180)")

        # --- 출/도착지, 경유지 검증 ---
        type_text = self.panel.type.currentText()
        row_count = self.panel.table.rowCount()

        # 1) 출/도착지 중복 타입 검증
        if type_text in [START, END]:
            for row in range(row_count):
                cell = self.panel.table.item(row, 0)
                if cell and cell.text() == type_text:
                    return validate(f"{type_text}는 이미 등록되어 있습니다.")

        # 2) 출/도착지 좌표 동일 검증
        if type_text == END:
            for row in range(row_count):
                cell = self.panel.table.item(row, 0)
                if cell and cell.text() == START:
                    start_lat = float(self.panel.table.item(row, 2).text())
                    start_lon = float(self.panel.table.item(row, 3).text())
                    if start_lat == lat and start_lon == lon:
                        self.panel.lat.setStyleSheet("border: 1px solid red;")
                        self.panel.lon.setStyleSheet("border: 1px solid red;")
                        return validate("출발지와 도착지는 동일 할 수 없습니다.")
        elif type_text == START:
            for row in range(row_count):
                cell = self.panel.table.item(row, 0)
                if cell and cell.text() == END:
                    end_lat = float(self.panel.table.item(row, 2).text())
                    end_lon = float(self.panel.table.item(row, 3).text())
                    if end_lat == lat and end_lon == lon:
                        self.panel.lat.setStyleSheet("border: 1px solid red;")
                        self.panel.lon.setStyleSheet("border: 1px solid red;")
                        return validate("출발지와 도착지는 동일 할 수 없습니다.")

        # 3) 경유지 중복 좌표 경고 (입력 제한 X, 안내창)
        if type_text == VIA:
            for row in range(row_count):
                cell_lat = float(self.panel.table.item(row, 2).text())
                cell_lon = float(self.panel.table.item(row, 3).text())
                if cell_lat == lat and cell_lon == lon:
                    QMessageBox.information(self.panel, "안내", "이미 동일한 경유지가 존재합니다.")
                    break

        # --- 행 위치 결정 ---
        if type_text == START:
            row = 0
        elif type_text == END:
            row = row_count
        else:
            row = self.panel.table.rowCount()
            if row_count > 0:
                last_cell = self.panel.table.item(row_count - 1, 0)
                if last_cell and last_cell.text() == END:
                    row -= 1

        self.panel.table.insertRow(row)

        # --- 행 등록 ---
        self.panel.table.setItem(row, 0, QTableWidgetItem(type_text))
        self.panel.table.setItem(row, 1, QTableWidgetItem(self.panel.port.text()))
        self.panel.table.setItem(row, 2, QTableWidgetItem(self.panel.lat.text()))
        self.panel.table.setItem(row, 3, QTableWidgetItem(self.panel.lon.text()))

        # 입력창 초기화
        self.panel.lat.setStyleSheet("")
        self.panel.lon.setStyleSheet("")
        self.panel.type.setCurrentIndex(0)
        self.panel.port.setText("-")
        self.panel.lat.clear()
        self.panel.lon.clear()

        # 지도 즉시 반영
        payload = self._build_payload()
        # print('[경유지 입력 페이로드 업데이트 (디버깅)]',payload)
        if payload is not None:
            self.panel.waypointsChanged.emit(payload)

    # 행 삭제
    def delete_selected(self):
        selected = self.panel.table.currentRow()
        if selected >= 0:
            self.panel.table.removeRow(selected)
            # 지도 즉시 반영
            payload = self._build_payload()
            if payload is not None:
                self.panel.waypointsChanged.emit(payload)

    # 초기화
    def clear_table(self) -> bool:
        answer = QMessageBox.question(
            self.panel, "안내", "정말 모든 경유지를 삭제하시겠습니까?",
            QMessageBox.Yes | QMessageBox.No, QMessageBox.No
        )
        if answer != QMessageBox.Yes:
            return False

        self.panel.table.setRowCount(0)
        QMessageBox.information(self.panel, "안내", "모든 경유지가 삭제되었습니다.")
        self.panel.waypointsChanged.emit({
            "depart_time": self.panel.depart_time.dateTime().toString("yyyy-MM-dd HH:mm:ss"),
            "waypoints": []
        })
        return True
    
    # 경로 탐색
    def emit_route(self):
        # --- 경고창 ---
        def validate(message):
            QMessageBox.warning(self.panel, "안내", message)
            return

        table = self.panel.table
        row_count = table.rowCount()
        if row_count == 0:
            return validate("입력된 경유지가 없습니다.")

        # 출/도착지 존재 검증
        has_start = any(table.item(r, 0) and table.item(r, 0).text() == START for r in range(row_count))
        has_end   = any(table.item(r, 0) and table.item(r, 0).text() == END for r in range(row_count))
        if not has_start or not has_end:
            missing = []
            if not has_start: missing.append(START)
            if not has_end:   missing.append(END)
            return validate(f"{', '.join(missing)}는 필수로 등록되어야 합니다.")

        # 출발 시각 검증
        dt = self.panel.depart_time.dateTime()
        if not dt.isValid():
            return validate("출발 시각을 선택해주세요.")
        depart_time = dt.toString("yyyy-MM-dd HH:mm:ss")

        # 경유지
        waypoint_data = []

        for r in range(self.panel.table.rowCount()):
            it_type = self.panel.table.item(r, 0)
            it_port = self.panel.table.item(r, 1)
            it_lat  = self.panel.table.item(r, 2)
            it_lon  = self.panel.table.item(r, 3)

            waypoint = {
                "type": it_type.text() if it_type else "",
                "port": it_port.text() if it_port else "",
                "lat": float(it_lat.text()) if it_lat else 0.0,
                "lon": float(it_lon.text()) if it_lon else 0.0
            }

            waypoint_data.append(waypoint)

        data = {
            "depart_time": depart_time,
            "waypoints": waypoint_data
        }

        print("[WaypointPanel] 경로 탐색 요청 전달 데이터", data)
        self.panel.routeRequested.emit(data)

    # 항구 검색
    def open_port_search(self):
        import os
        from .port_search_dialog import PortSearchDialog

        base_dir = os.path.dirname(os.path.dirname(__file__))
        csv_path = os.path.join(base_dir, "ui_data", "PortList.csv")

        dialog = PortSearchDialog(csv_path, self.panel)

        if dialog.exec():
            p = dialog.selected_port
            if p:
                self.panel.port.setText(p["name"])
                self.panel.lat.setText(str(p["lat"]))
                self.panel.lon.setText(str(p["lon"]))
    