#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Ship Routing Algorithm - Python Integration Test
"""

import os
import sys
from pathlib import Path

# ================================================================
# 1. 환경 설정 및 모듈 임포트
# ================================================================

def setup_environment():
    """DLL 경로 및 Python 모듈 경로 설정"""
    
    # DLL 경로 설정
    try:
        python_base_dir = sys.base_prefix 
        project_root = os.getcwd()
        lib_dir = os.path.join(project_root, "Lib")
        VCPKG_BIN = os.getenv("VCPKG_BIN", r"C:\vcpkg\installed\x64-windows\bin")
        
        # ✨ ShipDynamics.dll 경로
        ship_dynamics_dir = os.path.join(project_root, "core_engine", "algorithm", "algorithm", "data", "dll")
        
        # ✨ PATH 환경 변수에 추가 (C++ LoadLibrary가 찾을 수 있도록)
        if os.path.exists(ship_dynamics_dir):
            current_path = os.environ.get('PATH', '')
            if ship_dynamics_dir not in current_path:
                os.environ['PATH'] = ship_dynamics_dir + os.pathsep + current_path
                print(f"✅ PATH에 DLL 경로 추가: {ship_dynamics_dir}")

        if sys.version_info >= (3, 8) and os.name == 'nt':
            os.add_dll_directory(python_base_dir)
            if os.path.exists(lib_dir):
                os.add_dll_directory(lib_dir)
            if os.path.exists(VCPKG_BIN):
                os.add_dll_directory(VCPKG_BIN)
            # ✨ ShipDynamics.dll 경로 추가
            if os.path.exists(ship_dynamics_dir):
                os.add_dll_directory(ship_dynamics_dir)
                print(f"✅ add_dll_directory: {ship_dynamics_dir}")
            else:
                print(f"⚠️  ShipDynamics.dll 폴더 없음: {ship_dynamics_dir}")
        
        print("✅ DLL 경로 설정 완료")
        
    except Exception as e:
        print(f"⚠️  DLL 경로 설정 경고: {e}")
    
    # Python 모듈 경로 추가
    module_search_paths = [
        "core_engine/algorithm/algorithm/build/Debug",           # CMake 일반 빌드
        "core_engine/algorithm/algorithm/build/Release",
        "core_engine/algorithm/algorithm/out/build/x64-Debug",  # Visual Studio CMake
        "core_engine/algorithm/algorithm/out/build/x64-Release",
    ]
    
    for path in module_search_paths:
        full_path = os.path.join(os.getcwd(), path)
        if os.path.exists(full_path):
            sys.path.insert(0, full_path)
            print(f"✅ 모듈 경로 추가: {path}")

def import_module():
    """algorithm_module 임포트"""
    try:
        import algorithm_module
        print(f"✅ 모듈 임포트 성공: {algorithm_module.__file__}")
        return algorithm_module
    except ImportError as e:
        print(f"\n❌ 모듈 임포트 실패: {e}")
        print("\n가능한 원인:")
        print("  1. Python 모듈이 빌드되지 않았습니다")
        print("  2. CMake에서 algorithm_module 타겟 빌드 필요")
        print("  3. 빌드 위치를 확인하세요:")
        print("     - core_engine/algorithm/algorithm/build/Debug/")
        print("     - core_engine/algorithm/algorithm/out/build/x64-Debug/")
        sys.exit(1)

# ================================================================
# 2. 데이터 파일 경로 찾기
# ================================================================

def find_data_directory():
    """데이터 디렉토리 자동 탐색"""
    
    script_dir = Path(__file__).parent.absolute()
    
    # 후보 경로들 (통합 구조)
    candidates = [
        script_dir / "core_engine" / "algorithm" / "algorithm" / "data",
        script_dir / "data",  # 혹시 루트에 복사했다면
    ]
    
    for candidate in candidates:
        gebco = candidate / "gebco" / "GEBCO_2024_sub_ice_topo.nc"
        gshhs = candidate / "gshhs" / "GSHHS_i_L1.shp"
        
        if gebco.exists() and gshhs.exists():
            print(f"✅ 데이터 폴더 발견: {candidate}")
            return {
                'data_dir': str(candidate),
                'gebco': str(gebco),
                'gshhs': str(gshhs),
                'weather': str(candidate / "weather")
            }
    
    print("\n❌ 데이터 폴더를 찾을 수 없습니다!")
    print("\n확인 사항:")
    print("  1. 데이터 파일 위치:")
    print("     core_engine/algorithm/algorithm/data/gebco/GEBCO_2024_sub_ice_topo.nc")
    print("     core_engine/algorithm/algorithm/data/gshhs/GSHHS_i_L1.shp")
    print("  2. 현재 스크립트 실행 위치:", script_dir)
    
    return None

# ================================================================
# 3. 메인 테스트
# ================================================================

def run_test(algorithm_module):
    """메인 테스트 함수"""
    
    print("\n" + "="*60)
    print("Ship Router Python Integration Test")
    print("="*60)
    
    # Step 1: 데이터 경로 찾기
    print("\n[Step 1] 데이터 파일 검색...")
    data_paths = find_data_directory()
    if not data_paths:
        return False
    
    print(f"  GEBCO: {data_paths['gebco']}")
    print(f"  GSHHS: {data_paths['gshhs']}")
    print(f"  Weather: {data_paths['weather']}")
    
    # Step 2: ShipRouter 생성 및 초기화
    print("\n[Step 2] ShipRouter 초기화...")
    router = algorithm_module.ShipRouter()
    
    success = router.initialize(data_paths['gebco'], data_paths['gshhs'])
    
    if not success:
        print("❌ 초기화 실패")
        return False
    
    print("✅ 초기화 성공")
    
    # Step 3: 날씨 데이터 로딩 (선택적)
    print("\n[Step 3] 날씨 데이터 로딩 (선택적)...")
    try:
        router.load_weather_data(data_paths['weather'])
        print("✅ 날씨 데이터 로딩 시도 완료")
    except Exception as e:
        print(f"⚠️  날씨 데이터 로딩 실패 (계속 진행): {e}")
    
    # Step 4: 웨이포인트 설정
    print("\n[Step 4] 웨이포인트 설정...")
    
    # 테스트 케이스: 부산 -> 제주
    waypoints = [
        algorithm_module.GeoCoordinate(35.0994, 129.0336),  # 부산
        algorithm_module.GeoCoordinate(33.4996, 126.5312),  # 제주
    ]
    
    print(f"  웨이포인트 1: 부산 ({waypoints[0].latitude}, {waypoints[0].longitude})")
    print(f"  웨이포인트 2: 제주 ({waypoints[1].latitude}, {waypoints[1].longitude})")
    
    # Step 5: VoyageConfig 설정 (선택적)
    print("\n[Step 5] 항해 설정...")
    try:
        config = algorithm_module.VoyageConfig()
        config.ship_speed_mps = 8.0
        config.draft_m = 10.0
        config.grid_cell_size_km = 5.0
        config.calculate_shortest = True
        config.calculate_optimized = True
        
        print(f"  선박 속도: {config.ship_speed_mps} m/s")
        print(f"  흘수: {config.draft_m} m")
        print(f"  그리드 크기: {config.grid_cell_size_km} km")
        
        use_config = True
    except AttributeError:
        print("  ⚠️ VoyageConfig 미지원 - 기본 설정 사용")
        config = None
        use_config = False
    
    # Step 6: 경로 계산
    print("\n[Step 6] 경로 계산 중...")
    print("  (이 작업은 몇 초에서 몇 분 소요될 수 있습니다)")
    
    try:
        if use_config:
            result = router.calculate_route(waypoints, config)
        else:
            result = router.calculate_route(waypoints)
    except Exception as e:
        print(f"❌ 경로 계산 중 오류: {e}")
        import traceback
        traceback.print_exc()
        return False
    
    # Step 7: 결과 출력
    print("\n[Step 7] 결과 분석...")
    
    if not result.success:
        print(f"\n❌ 경로 계산 실패: {result.error_message}")
        
        # 스냅핑 정보 출력
        if hasattr(result, 'snapping_info') and result.snapping_info:
            print("\n웨이포인트 스냅핑 상태:")
            for i, info in enumerate(result.snapping_info):
                print(f"  웨이포인트 {i+1}:")
                if hasattr(info, 'status'):
                    print(f"    상태: {info.status}")
                if hasattr(info, 'failure_reason') and info.failure_reason:
                    print(f"    실패 원인: {info.failure_reason}")
        
        return False
    
    print("\n✅ 경로 계산 성공!")
    
    # ================================================================
    # 스냅핑 정보 상세 출력
    # ================================================================
    if hasattr(result, 'snapping_info') and result.snapping_info:
        print("\n" + "="*60)
        print("📍 웨이포인트 스냅핑 상세 정보")
        print("="*60)
        for i, info in enumerate(result.snapping_info):
            print(f"\n[웨이포인트 {i+1}]")
            print(f"  원본 좌표: ({info.original.latitude:.6f}, {info.original.longitude:.6f})")
            print(f"  스냅 상태: {info.status}")
            
            if hasattr(info, 'was_snapped'):
                print(f"  스냅 여부: {info.was_snapped}")
            
            if info.status == algorithm_module.SnappingStatus.SNAPPED:
                print(f"  스냅 좌표: ({info.snapped.latitude:.6f}, {info.snapped.longitude:.6f})")
                print(f"  스냅 거리: {info.snapping_distance_km:.4f} km")
            
            if hasattr(info, 'failure_reason') and info.failure_reason:
                print(f"  실패 원인: {info.failure_reason}")
    
    # ================================================================
    # 최단 경로 상세 출력
    # ================================================================
    if hasattr(result, 'shortest_path') and result.shortest_path.success:
        print("\n" + "="*60)
        print("📊 최단 경로 (Shortest Path) - 요약")
        print("="*60)
        sp = result.shortest_path.summary
        print(f"  총 거리:        {sp.total_distance_km:.4f} km")
        print(f"  총 시간:        {sp.total_time_hours:.4f} hours ({sp.total_time_hours*60:.2f} min)")
        print(f"  총 연료:        {sp.total_fuel_kg:.4f} kg ({sp.total_fuel_kg/1000:.6f} tons)")
        print(f"  평균 속도:      {sp.average_speed_mps:.4f} m/s ({sp.average_speed_mps*1.94384:.2f} knots)")
        print(f"  평균 연료율:    {sp.average_fuel_rate_kg_per_hour:.4f} kg/h")
        print(f"  총 경로점 수:   {len(result.shortest_path.path_details)}")
        
        # 경로점 상세 (처음 10개 + 마지막 10개)
        print("\n" + "-"*60)
        print("경로점 상세 정보 (처음 10개)")
        print("-"*60)
        print(f"{'No':>4} {'Lat':>10} {'Lon':>11} {'Dist(km)':>10} {'Time(h)':>9} {'Fuel(kg)':>10} {'Speed(m/s)':>11} {'Heading':>8}")
        print("-"*60)
        
        for i, point in enumerate(result.shortest_path.path_details[:10]):
            heading = point.heading_degrees if hasattr(point, 'heading_degrees') else 0.0
            print(f"{i+1:4d} {point.position.latitude:10.6f} {point.position.longitude:11.6f} "
                  f"{point.cumulative_distance_km:10.4f} {point.cumulative_time_hours:9.4f} "
                  f"{point.cumulative_fuel_kg:10.4f} {point.speed_mps:11.4f} {heading:8.2f}")
        
        if len(result.shortest_path.path_details) > 20:
            print(f"  ... ({len(result.shortest_path.path_details) - 20} points omitted)")
            
            print("\n경로점 상세 정보 (마지막 10개)")
            print("-"*60)
            for i, point in enumerate(result.shortest_path.path_details[-10:], 
                                     start=len(result.shortest_path.path_details)-10):
                heading = point.heading_degrees if hasattr(point, 'heading_degrees') else 0.0
                print(f"{i+1:4d} {point.position.latitude:10.6f} {point.position.longitude:11.6f} "
                      f"{point.cumulative_distance_km:10.4f} {point.cumulative_time_hours:9.4f} "
                      f"{point.cumulative_fuel_kg:10.4f} {point.speed_mps:11.4f} {heading:8.2f}")
        
        # 날씨 정보 (첫 번째 점)
        if len(result.shortest_path.path_details) > 0:
            first_point = result.shortest_path.path_details[0]
            if hasattr(first_point, 'weather'):
                print("\n📡 첫 번째 경로점의 날씨 정보:")
                w = first_point.weather
                print(f"  풍향: {w.windDir:.2f}°, 풍속: {w.windSpd:.2f} m/s")
                print(f"  조류 방향: {w.currDir:.2f}°, 조류 속도: {w.currSpd:.2f} m/s")
                print(f"  파향: {w.waveDir:.2f}°, 파고: {w.waveHgt:.2f} m, 파주기: {w.wavePrd:.2f} s")
    
    # ================================================================
    # 최적 경로 상세 출력
    # ================================================================
    if hasattr(result, 'optimized_path') and result.optimized_path.success:
        print("\n" + "="*60)
        print("📊 최적 경로 (Optimized Path) - 요약")
        print("="*60)
        op = result.optimized_path.summary
        print(f"  총 거리:        {op.total_distance_km:.4f} km")
        print(f"  총 시간:        {op.total_time_hours:.4f} hours ({op.total_time_hours*60:.2f} min)")
        print(f"  총 연료:        {op.total_fuel_kg:.4f} kg ({op.total_fuel_kg/1000:.6f} tons)")
        print(f"  평균 속도:      {op.average_speed_mps:.4f} m/s ({op.average_speed_mps*1.94384:.2f} knots)")
        print(f"  평균 연료율:    {op.average_fuel_rate_kg_per_hour:.4f} kg/h")
        print(f"  총 경로점 수:   {len(result.optimized_path.path_details)}")
        
        # 경로점 상세 (처음 10개 + 마지막 10개)
        print("\n" + "-"*60)
        print("경로점 상세 정보 (처음 10개)")
        print("-"*60)
        print(f"{'No':>4} {'Lat':>10} {'Lon':>11} {'Dist(km)':>10} {'Time(h)':>9} {'Fuel(kg)':>10} {'Speed(m/s)':>11} {'Heading':>8}")
        print("-"*60)
        
        for i, point in enumerate(result.optimized_path.path_details[:10]):
            heading = point.heading_degrees if hasattr(point, 'heading_degrees') else 0.0
            print(f"{i+1:4d} {point.position.latitude:10.6f} {point.position.longitude:11.6f} "
                  f"{point.cumulative_distance_km:10.4f} {point.cumulative_time_hours:9.4f} "
                  f"{point.cumulative_fuel_kg:10.4f} {point.speed_mps:11.4f} {heading:8.2f}")
        
        if len(result.optimized_path.path_details) > 20:
            print(f"  ... ({len(result.optimized_path.path_details) - 20} points omitted)")
            
            print("\n경로점 상세 정보 (마지막 10개)")
            print("-"*60)
            for i, point in enumerate(result.optimized_path.path_details[-10:], 
                                     start=len(result.optimized_path.path_details)-10):
                heading = point.heading_degrees if hasattr(point, 'heading_degrees') else 0.0
                print(f"{i+1:4d} {point.position.latitude:10.6f} {point.position.longitude:11.6f} "
                      f"{point.cumulative_distance_km:10.4f} {point.cumulative_time_hours:9.4f} "
                      f"{point.cumulative_fuel_kg:10.4f} {point.speed_mps:11.4f} {heading:8.2f}")
        
        # 날씨 정보 (첫 번째 점)
        if len(result.optimized_path.path_details) > 0:
            first_point = result.optimized_path.path_details[0]
            if hasattr(first_point, 'weather'):
                print("\n📡 첫 번째 경로점의 날씨 정보:")
                w = first_point.weather
                print(f"  풍향: {w.windDir:.2f}°, 풍속: {w.windSpd:.2f} m/s")
                print(f"  조류 방향: {w.currDir:.2f}°, 조류 속도: {w.currSpd:.2f} m/s")
                print(f"  파향: {w.waveDir:.2f}°, 파고: {w.waveHgt:.2f} m, 파주기: {w.wavePrd:.2f} s")
    
    # ================================================================
    # 비교 분석
    # ================================================================
    if (hasattr(result, 'shortest_path') and result.shortest_path.success and
        hasattr(result, 'optimized_path') and result.optimized_path.success):
        
        print("\n" + "="*60)
        print("📈 최단 경로 vs 최적 경로 비교")
        print("="*60)
        
        sp = result.shortest_path.summary
        op = result.optimized_path.summary
        
        # 거리 비교
        dist_diff = op.total_distance_km - sp.total_distance_km
        dist_pct = (dist_diff / sp.total_distance_km * 100) if sp.total_distance_km > 0 else 0
        print(f"\n거리:")
        print(f"  최단 경로:  {sp.total_distance_km:.4f} km")
        print(f"  최적 경로:  {op.total_distance_km:.4f} km")
        print(f"  차이:       {dist_diff:+.4f} km ({dist_pct:+.2f}%)")
        
        # 시간 비교
        time_diff = op.total_time_hours - sp.total_time_hours
        time_pct = (time_diff / sp.total_time_hours * 100) if sp.total_time_hours > 0 else 0
        print(f"\n시간:")
        print(f"  최단 경로:  {sp.total_time_hours:.4f} hours")
        print(f"  최적 경로:  {op.total_time_hours:.4f} hours")
        print(f"  차이:       {time_diff:+.4f} hours ({time_pct:+.2f}%)")
        
        # 연료 비교
        fuel_diff = op.total_fuel_kg - sp.total_fuel_kg
        fuel_pct = (fuel_diff / sp.total_fuel_kg * 100) if sp.total_fuel_kg > 0 else 0
        print(f"\n연료:")
        print(f"  최단 경로:  {sp.total_fuel_kg:.4f} kg ({sp.total_fuel_kg/1000:.6f} tons)")
        print(f"  최적 경로:  {op.total_fuel_kg:.4f} kg ({op.total_fuel_kg/1000:.6f} tons)")
        print(f"  차이:       {fuel_diff:+.4f} kg ({fuel_pct:+.2f}%)")
        
        # 결론
        print(f"\n💡 결론:")
        if abs(fuel_diff) < 0.01:
            print(f"  ⚠️  두 경로가 동일합니다 (ShipDynamics.dll 미작동 가능성)")
        elif fuel_diff < 0:
            print(f"  ✅ 최적 경로가 {abs(fuel_diff):.4f} kg ({abs(fuel_diff)/1000:.6f} tons) 연료 절감!")
        else:
            print(f"  ⚠️  최단 경로가 {fuel_diff:.4f} kg 더 효율적 (알고리즘 조정 필요)")
    
    print("\n" + "="*60)
    print("테스트 완료!")
    print("="*60)
    
    return True

# ================================================================
# 4. 실행
# ================================================================

if __name__ == "__main__":
    try:
        # 환경 설정
        setup_environment()
        
        # 모듈 임포트
        algorithm_module = import_module()
        
        # 테스트 실행
        success = run_test(algorithm_module)
        
        # 종료 코드
        sys.exit(0 if success else 1)
        
    except KeyboardInterrupt:
        print("\n\n⚠️  사용자에 의해 중단됨")
        sys.exit(1)
    except Exception as e:
        print(f"\n❌ 예상치 못한 오류: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)