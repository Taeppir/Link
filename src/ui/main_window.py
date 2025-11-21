from PySide6.QtCore import Qt
from PySide6.QtWidgets import QMainWindow, QWidget, QVBoxLayout, QSplitter
from src.ui import SideTools, MapPanel
from .waypoint import WaypointPanel
from .report import ReportPanel
from .side_tools import SideTools
from .map.map_panel import MapPanel
from .waypoint.waypoint_panel import WaypointPanel

try:
    import algorithm_module
except ImportError as e:
    print(f"[MainWindow] !!! CRITICAL ERROR in main_window.py: Failed to import algorithm_module: {e}")
    algorithm_module = None

class MainWindow(QMainWindow):
    def __init__(self, router) -> None:
        super().__init__()
        self.setWindowTitle("Route Planner")
        self.resize(1600, 900)
        self.router = router

        central = QWidget(self)
        self.setCentralWidget(central)
        root = QVBoxLayout(central)

        # 상단 UI
        top = QSplitter(Qt.Horizontal, self)
        self.tools = SideTools()
        self.map = MapPanel()
        self.waypoints = WaypointPanel()
        top.addWidget(self.tools)
        top.addWidget(self.map)
        top.addWidget(self.waypoints)
        top.setStretchFactor(0, 0)
        top.setStretchFactor(1, 5.5)
        top.setStretchFactor(2, 4.5)

        # 하단 UI
        self.report = ReportPanel()
        root_split = QSplitter(Qt.Vertical, self)
        root_split.addWidget(top)
        root_split.addWidget(self.report)
        root_split.setSizes([600, 400])
        root.addWidget(root_split)

        # 오버레이 토글
        self.tools.depthToggled.connect(lambda on: self.map.toggle_overlay("Seamarks", on))
        self.tools.coastlineToggled.connect(lambda on: self.map.toggle_overlay("Coast Lines", on))
        self.tools.weatherToggled.connect(lambda on: self.map.setLayerInfo(f"날씨 레이어 {'ON' if on else 'OFF'}"))

        # 경유지 → 지도 반영
        # 1) C++ 모듈로 경로 탐색 요청 및 출력 패널 활성화
        self.waypoints.routeRequested.connect(self._on_route)
        self.waypoints.routeRequested.connect(self.report.slots.update_report) 
        # 2) 경유지 즉시 지도 갱신
        self.waypoints.waypointsChanged.connect(self._on_waypoints_changed)
        self.waypoints.attach_map_panel(self.map)

        # 기본 켜기
        self.tools.btn_depth.setChecked(False)
        self.tools.btn_coast.setChecked(False)

    # 지도 즉시 갱신
    def _on_waypoints_changed(self, payload: dict):
        self.map.set_waypoints(payload)
        self.map.setLayerInfo("경유지 업데이트")

    # C++ 모듈 경로 탐색 요청
    def _on_route(self, payload: dict) -> None:
        if not self.router:
            return

        # C++ 함수 호출 준비
        py_waypoints_data = payload.get('waypoints', [])
        cpp_waypoints = []

        if not py_waypoints_data:
            self.map.set_waypoints(payload) 
            return 

        for wp in py_waypoints_data:
            try:
                lat = wp.get('lat')
                long = wp.get('lon')

                if lat is None or long is None:
                    print(f"[MainWindow] 경고: 잘못된 형식의 경유지 데이터 건너<0xEB><0x9C><0x84>뜀 - {wp}")
                    continue

                coord = algorithm_module.GeoCoordinate(
                    latitude = float(lat),
                    longitude = float(long)
                )
                cpp_waypoints.append(coord)  

            except (IndexError, ValueError, TypeError) as e:
                print(f"[MainWindow] 오류: 경유지 데이터 변환 중 문제 발생 - {wp}, 오류: {e}")

        if len(cpp_waypoints) < 2:
            print("[MainWindow] 오류: 유효한 경유지가 2개 미만입니다. 경로 계산을 할 수 없습니다.")
            self.map.set_waypoints(payload) 
            return

        print('[python >> c++ 모듈 데이터]',cpp_waypoints)

        try:
            result = self.router.calculate_shortest_path_analysis(
                waypoints_geo = cpp_waypoints,
                ship_speed_mps=8.0
            )
        except Exception as e:
            print(f"[MainWindow] !!! 치명적 오류: C++ 함수 호출 중 예외 발생: {e}")
            return

        print("\n[MainWindow] --- C++ 계산 결과 ---")
        if result.success:
            print("  성공!")
            print(f"  총 거리: {result.total_distance_km:.2f} km")
            print(f"  총 시간: {result.total_time_hours:.2f} hours")
            print(f"  총 연료: {result.total_fuel_kg:.2f} kg (약 {(result.total_fuel_kg / 1000.0):.2f} ton)")
            print(f"  경로 지점 수: {len(result.full_path_geo)}")
            print(f"  최적 총 거리: {result.optimal_distance_km:.2f} km")
            print(f"  최적 총 시간: {result.optimal_time_hours:.2f} hours")
            print(f"  최적 총 연료: {result.optimal_fuel_kg:.2f} kg (약 {(result.total_fuel_kg / 1000.0):.2f} ton)")
            if result.full_path_geo:
                print(f"  경로 시작점: {result.full_path_geo[0]}")
                print(f"  경로 끝점: {result.full_path_geo[-1]}")

           
            self.map.set_waypoints(payload)

            report_data = {
                'distance': result.total_distance_km,
                'time': result.total_time_hours,
                'fuel': result.total_fuel_kg / 1000.0,
                'path_points': [{'lat': p.latitude, 'lon': p.longitude} for p in result.full_path_geo],

                'optimal_distance': result.optimal_distance_km,
                'optimal_time': result.optimal_time_hours,
                'optimal_fuel': result.optimal_fuel_kg / 1000.0
            }
            self.report.slots.update_report(report_data)

            self.map.setLayerInfo("탐색 결과 표시")
        else:
            print(f"[MainWindow] 경로 탐색 실패: {result.error_message}")
            self.map.set_waypoints(payload) 
            self.map.setLayerInfo(f"경로 탐색 실패: {result.error_message}")