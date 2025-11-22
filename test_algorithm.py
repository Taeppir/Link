#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Ship Routing Algorithm - Python Integration Test
커맨드라인으로 waypoints를 주입받아 실행

사용 예시:
    python test_algorithm.py --waypoints "35.0994,129.0336" "33.4996,126.5312"
    python test_algorithm.py -w "37.5665,126.9780" "35.0994,129.0336" "33.4996,126.5312"
    python test_algorithm.py  # 기본값: 부산 -> 제주
"""

import os
import sys
import argparse
from pathlib import Path

# ================================================================
# 1. 환경 설정 및 모듈 임포트
# ================================================================

def setup_environment():
    proj_lib = r"C:\vcpkg\installed\x64-windows\share\proj"
    if os.path.exists(proj_lib):
        os.environ['PROJ_LIB'] = proj_lib
        print(f"✅ PROJ_LIB 설정: {proj_lib}")
    
    
    """DLL 경로 및 Python 모듈 경로 설정"""
    
    # DLL 경로 설정
    try:
        python_base_dir = sys.base_prefix 
        project_root = os.getcwd()
        lib_dir = os.path.join(project_root, "Lib")
        VCPKG_BIN = os.getenv("VCPKG_BIN", r"C:\vcpkg\installed\x64-windows\bin")
        
        # ✨ ShipDynamics.dll 경로 (새로운 구조: LINK/data/dll/)
        ship_dynamics_dir = os.path.join(project_root, "data", "dll")
        
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
    """데이터 디렉토리 자동 탐색 (새로운 구조: LINK/data/)"""
    
    script_dir = Path(__file__).parent.absolute()
    
    # 새로운 통합 구조: LINK/data/
    data_dir = script_dir / "data"
    gebco = data_dir / "gebco" / "GEBCO_2024_sub_ice_topo.nc"
    gshhs = data_dir / "gshhs" / "GSHHS_i_L1.shp"
    
    if gebco.exists() and gshhs.exists():
        print(f"✅ 데이터 폴더 발견: {data_dir}")
        return {
            'data_dir': str(data_dir),
            'gebco': str(gebco),
            'gshhs': str(gshhs),
            'weather': str(data_dir / "weather")
        }
    
    print("\n❌ 데이터 폴더를 찾을 수 없습니다!")
    print("\n확인 사항:")
    print("  1. 데이터 파일 위치:")
    print(f"     {gebco}")
    print(f"     {gshhs}")
    print("  2. 현재 스크립트 실행 위치:", script_dir)
    print("\n예상 구조:")
    print("  LINK/")
    print("  ├── data/")
    print("  │   ├── gebco/GEBCO_2024_sub_ice_topo.nc")
    print("  │   ├── gshhs/GSHHS_i_L1.shp")
    print("  │   ├── weather/*.bin")
    print("  │   └── dll/ShipDynamics.dll")
    print("  └── test_algorithm.py (현재 스크립트)")
    
    return None

# ================================================================
# 3. 커맨드라인 인자 파싱
# ================================================================

def parse_arguments():
    """커맨드라인 인자 파싱"""
    parser = argparse.ArgumentParser(
        description='Ship Routing Algorithm - Integration Test',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog='''
사용 예시:
  # 기본값 (부산 -> 제주)
  python test_algorithm.py
  
  # 2개 웨이포인트 (부산 -> 제주)
  python test_algorithm.py --waypoints "35.0994,129.0336" "33.4996,126.5312"
  
  # 3개 웨이포인트 (서울 -> 부산 -> 제주)
  python test_algorithm.py -w "37.5665,126.9780" "35.0994,129.0336" "33.4996,126.5312"
  
  # 고급 설정
  python test_algorithm.py -w "35.0994,129.0336" "33.4996,126.5312" \\
      --speed 10.0 --draft 12.0 --grid-size 3.0
        '''
    )
    
    parser.add_argument(
        '-w', '--waypoints',
        nargs='+',
        help='웨이포인트 좌표 (위도,경도 형식). 예: "35.0994,129.0336" "33.4996,126.5312"'
    )
    
    parser.add_argument(
        '--speed',
        type=float,
        default=8.0,
        help='선박 속도 (m/s). 기본값: 8.0'
    )
    
    parser.add_argument(
        '--draft',
        type=float,
        default=10.0,
        help='선박 흘수 (m). 기본값: 10.0'
    )
    
    parser.add_argument(
        '--grid-size',
        type=float,
        default=5.0,
        help='그리드 셀 크기 (km). 기본값: 5.0'
    )
    
    args = parser.parse_args()
    
    # 웨이포인트 파싱
    if args.waypoints:
        waypoints = []
        for wp in args.waypoints:
            try:
                lat, lon = map(float, wp.split(','))
                waypoints.append((lat, lon))
            except ValueError:
                print(f"❌ 잘못된 웨이포인트 형식: {wp}")
                print("   올바른 형식: \"위도,경도\" (예: \"35.0994,129.0336\")")
                sys.exit(1)
        
        if len(waypoints) < 2:
            print("❌ 최소 2개 이상의 웨이포인트가 필요합니다")
            sys.exit(1)
    else:
        # 기본값: 부산 -> 제주 -> 오키나와 -> 가오슝 -> 싱가포르
        print("\n⚠️  웨이포인트 미지정. 기본 경로 사용")
        print("   (부산 → 제주 → 오키나와 → 가오슝 → 싱가포르)")
        waypoints = [
            (35.0994, 129.0336),  # WP0: 부산항
            (33.4996, 126.5312),  # WP1: 제주도 제주시청
            (26.2124, 127.6809),  # WP2: 일본 오키나와 나하시청
            (22.6273, 120.3014),  # WP3: 대만 가오슝시청
            (1.2903, 103.8520),   # WP4: 싱가포르
        ]
    
    return waypoints, args

# ================================================================
# 4. 메인 테스트
# ================================================================

def run_test(algorithm_module, waypoints, config_args):
    """메인 테스트 함수"""
    
    print("\n" + "="*70)
    print("🚢 Ship Router - 경로 계산 테스트")
    print("="*70)
    
    # Step 1: 데이터 경로 찾기
    print("\n[1/6] 데이터 파일 검색...")
    data_paths = find_data_directory()
    if not data_paths:
        return False
    
    print(f"  ✓ GEBCO: {data_paths['gebco']}")
    print(f"  ✓ GSHHS: {data_paths['gshhs']}")
    print(f"  ✓ Weather: {data_paths['weather']}")
    
    # Step 2: ShipRouter 생성 및 초기화
    print("\n[2/6] ShipRouter 초기화...")
    router = algorithm_module.ShipRouter()
    
    success = router.initialize(data_paths['gebco'], data_paths['gshhs'])
    
    if not success:
        print("  ❌ 초기화 실패")
        return False
    
    print("  ✓ 초기화 성공")
    
    # Step 3: 날씨 데이터 로딩
    print("\n[3/6] 날씨 데이터 로딩...")
    try:
        router.load_weather_data(data_paths['weather'])
        print("  ✓ 날씨 데이터 로딩 완료")
    except Exception as e:
        print(f"  ⚠️  날씨 데이터 로딩 실패 (계속 진행): {e}")
    
    # Step 4: 웨이포인트 설정
    print(f"\n[4/6] 웨이포인트 설정 ({len(waypoints)}개)...")
    
    # 커맨드라인에서 받은 waypoints를 GeoCoordinate로 변환
    waypoint_objects = []
    for i, (lat, lon) in enumerate(waypoints, 1):
        waypoint_objects.append(algorithm_module.GeoCoordinate(lat, lon))
        print(f"  WP{i}: ({lat:.4f}, {lon:.4f})")
    
    # Step 5: VoyageConfig 설정
    print("\n[5/6] 항해 설정...")
    project_root = os.getcwd() 
    results_dir = os.path.join(project_root, "src", "ui", "results")
    
    os.makedirs(results_dir, exist_ok=True)
    print(f"📂 결과 저장 경로 설정됨: {results_dir}")
    
    try:
        config = algorithm_module.VoyageConfig()
        
        config.start_time_unix = 1577836800  # 2020-01-01 (날씨 데이터 시작 시간)
        config.calculate_shortest = True
        config.calculate_optimized = True
        config.output_path = str(results_dir)
        
        print(f"  ✓ 출발 시간: 2020-01-01")
        print(f"  ✓ 저장 경로: {config.output_path}")
        
        use_config = True
        
    except AttributeError:
        print("  ⚠️  VoyageConfig를 찾을 수 없습니다. 기본 설정으로 진행합니다.")
        use_config = False
    

    # Step 6: 경로 계산
    print("\n[6/6] 경로 계산 시작...")
    print("  (계산 중... 잠시만 기다려 주세요)")
    print("")
    try:
        if use_config:
            result = router.calculate_route(waypoint_objects, config)
        else:
            result = router.calculate_route(waypoint_objects)
        
        print("  ✅ 경로 계산 완료!")
    except Exception as e:
        print(f"  ❌ 경로 계산 실패: {e}")
        import traceback
        traceback.print_exc()
        return False
    
    # ================================================================
    # 결과 출력
    # ================================================================
    print("\n" + "="*70)
    print("📋 경로 계산 결과")
    print("="*70)
    
    # 전체 상태
    print(f"\n전체 성공 여부: {'✅ 성공' if result.success else '❌ 실패'}")
    
    # 웨이포인트 스냅 정보
    if hasattr(result, 'snapping_info') and len(result.snapping_info) > 0:
        print(f"\n【 웨이포인트 스냅 정보 】")
        print("─"*70)
        for i, info in enumerate(result.snapping_info, 1):
            print(f"\n  ▸ 웨이포인트 #{i}")
            print(f"    원본 좌표: ({info.original.latitude:.6f}, {info.original.longitude:.6f})")
            
            if hasattr(info, 'status'):
                status_map = {
                    algorithm_module.SnappingStatus.ALREADY_NAVIGABLE: "✅ 항해 가능",
                    algorithm_module.SnappingStatus.SNAPPED: "✅ 스냅 완료",
                    algorithm_module.SnappingStatus.FAILED: "❌ 스냅 실패"
                }
                print(f"    상태: {status_map.get(info.status, '알 수 없음')}")
            
            if info.status == algorithm_module.SnappingStatus.SNAPPED:
                print(f"    스냅 좌표: ({info.snapped.latitude:.6f}, {info.snapped.longitude:.6f})")
                print(f"    스냅 거리: {info.snapping_distance_km:.2f} km")
            
            if hasattr(info, 'failure_reason') and info.failure_reason:
                print(f"    실패 원인: {info.failure_reason}")
    
    # ================================================================
    # 최단 경로 상세 출력
    # ================================================================
    if hasattr(result, 'shortest_path') and result.shortest_path.success:
        print("\n" + "="*70)
        print("📊 최단 경로 (Shortest Path) - 결과")
        print("="*70)
        sp = result.shortest_path.summary
        print(f"\n【 경로 요약 】")
        print(f"  총 거리:        {sp.total_distance_km:.2f} km")
        print(f"  총 시간:        {sp.total_time_hours:.2f} hours ({sp.total_time_hours*60:.1f} min)")
        print(f"  총 연료 소비:   {sp.total_fuel_kg:.2f} kg")
        print(f"  평균 속도:      {sp.average_speed_mps:.2f} m/s ({sp.average_speed_mps*1.94384:.2f} knots)")
        print(f"  평균 연료율:    {sp.average_fuel_rate_kg_per_hour:.2f} kg/h")
        print(f"  총 경로점:      {len(result.shortest_path.path_details)}개")
        
        # 경로점 상세 (처음 5개만)
        print(f"\n【 경로점 상세 정보 】(처음 5개만 표시)")
        print("─"*70)
        print(f"{'순번':^6} {'위도':^12} {'경도':^12} {'누적거리':^10} {'누적시간':^10} {'누적연료':^10}")
        print(f"{'':^6} {'(deg)':^12} {'(deg)':^12} {'(km)':^10} {'(hour)':^10} {'(kg)':^10}")
        print("─"*70)
        
        for i, point in enumerate(result.shortest_path.path_details[:5], 1):
            print(f"{i:^6} {point.position.latitude:12.6f} {point.position.longitude:12.6f} "
                  f"{point.cumulative_distance_km:10.2f} {point.cumulative_time_hours:10.2f} "
                  f"{point.cumulative_fuel_kg:10.2f}")
        
        if len(result.shortest_path.path_details) > 10:
            print(f"{'...':^6} {'...':^12} {'...':^12} {'...':^10} {'...':^10} {'...':^10}")
            print(f"\n【 경로점 상세 정보 】(마지막 5개)")
            print("─"*70)
            for i, point in enumerate(result.shortest_path.path_details[-5:], 
                                     start=len(result.shortest_path.path_details)-4):
                print(f"{i:^6} {point.position.latitude:12.6f} {point.position.longitude:12.6f} "
                      f"{point.cumulative_distance_km:10.2f} {point.cumulative_time_hours:10.2f} "
                      f"{point.cumulative_fuel_kg:10.2f}")
        
        # 날씨 정보 (첫 번째 점)
        if len(result.shortest_path.path_details) > 0:
            first_point = result.shortest_path.path_details[0]
            if hasattr(first_point, 'weather'):
                print(f"\n【 출발점 날씨 정보 】")
                w = first_point.weather
                print(f"  풍향/풍속: {w.windDir:.1f}° / {w.windSpd:.2f} m/s")
                print(f"  조류: {w.currDir:.1f}° / {w.currSpd:.2f} m/s")
                print(f"  파향/파고/주기: {w.waveDir:.1f}° / {w.waveHgt:.2f} m / {w.wavePrd:.1f} s")
    
    # ================================================================
    # 최적 경로 상세 출력
    # ================================================================
    if hasattr(result, 'optimized_path') and result.optimized_path.success:
        print("\n" + "="*70)
        print("📊 최적 경로 (Optimized Path) - 결과")
        print("="*70)
        op = result.optimized_path.summary
        print(f"\n【 경로 요약 】")
        print(f"  총 거리:        {op.total_distance_km:.2f} km")
        print(f"  총 시간:        {op.total_time_hours:.2f} hours ({op.total_time_hours*60:.1f} min)")
        print(f"  총 연료 소비:   {op.total_fuel_kg:.2f} kg")
        print(f"  평균 속도:      {op.average_speed_mps:.2f} m/s ({op.average_speed_mps*1.94384:.2f} knots)")
        print(f"  평균 연료율:    {op.average_fuel_rate_kg_per_hour:.2f} kg/h")
        print(f"  총 경로점:      {len(result.optimized_path.path_details)}개")
        
        # 경로점 상세 (처음 5개만)
        print(f"\n【 경로점 상세 정보 】(처음 5개만 표시)")
        print("─"*70)
        print(f"{'순번':^6} {'위도':^12} {'경도':^12} {'누적거리':^10} {'누적시간':^10} {'누적연료':^10}")
        print(f"{'':^6} {'(deg)':^12} {'(deg)':^12} {'(km)':^10} {'(hour)':^10} {'(kg)':^10}")
        print("─"*70)
        
        for i, point in enumerate(result.optimized_path.path_details[:5], 1):
            print(f"{i:^6} {point.position.latitude:12.6f} {point.position.longitude:12.6f} "
                  f"{point.cumulative_distance_km:10.2f} {point.cumulative_time_hours:10.2f} "
                  f"{point.cumulative_fuel_kg:10.2f}")
        
        if len(result.optimized_path.path_details) > 10:
            print(f"{'...':^6} {'...':^12} {'...':^12} {'...':^10} {'...':^10} {'...':^10}")
            print(f"\n【 경로점 상세 정보 】(마지막 5개)")
            print("─"*70)
            for i, point in enumerate(result.optimized_path.path_details[-5:], 
                                     start=len(result.optimized_path.path_details)-4):
                print(f"{i:^6} {point.position.latitude:12.6f} {point.position.longitude:12.6f} "
                      f"{point.cumulative_distance_km:10.2f} {point.cumulative_time_hours:10.2f} "
                      f"{point.cumulative_fuel_kg:10.2f}")
        
        # 날씨 정보 (첫 번째 점)
        if len(result.optimized_path.path_details) > 0:
            first_point = result.optimized_path.path_details[0]
            if hasattr(first_point, 'weather'):
                print(f"\n【 출발점 날씨 정보 】")
                w = first_point.weather
                print(f"  풍향/풍속: {w.windDir:.1f}° / {w.windSpd:.2f} m/s")
                print(f"  조류: {w.currDir:.1f}° / {w.currSpd:.2f} m/s")
                print(f"  파향/파고/주기: {w.waveDir:.1f}° / {w.waveHgt:.2f} m / {w.wavePrd:.1f} s")
    
    # ================================================================
    # 비교 분석
    # ================================================================
    if (hasattr(result, 'shortest_path') and result.shortest_path.success and
        hasattr(result, 'optimized_path') and result.optimized_path.success):
        
        print("\n" + "="*70)
        print("📈 경로 비교 분석 (최적 경로 vs 최단 경로)")
        print("="*70)
        
        sp = result.shortest_path.summary
        op = result.optimized_path.summary
        
        print(f"\n{'항목':^12} {'최단 경로':>15} {'최적 경로':>15} {'차이':>15} {'비율':>10}")
        print("─"*70)
        
        # 거리 비교
        dist_diff = op.total_distance_km - sp.total_distance_km
        dist_pct = (dist_diff / sp.total_distance_km * 100) if sp.total_distance_km > 0 else 0
        print(f"{'거리':^12} {sp.total_distance_km:>13.2f} km {op.total_distance_km:>13.2f} km "
              f"{dist_diff:>+13.2f} km {dist_pct:>+9.2f}%")
        
        # 시간 비교
        time_diff = op.total_time_hours - sp.total_time_hours
        time_pct = (time_diff / sp.total_time_hours * 100) if sp.total_time_hours > 0 else 0
        print(f"{'시간':^12} {sp.total_time_hours:>13.2f} h  {op.total_time_hours:>13.2f} h  "
              f"{time_diff:>+13.2f} h  {time_pct:>+9.2f}%")
        
        # 연료 비교
        fuel_diff = op.total_fuel_kg - sp.total_fuel_kg
        fuel_pct = (fuel_diff / sp.total_fuel_kg * 100) if sp.total_fuel_kg > 0 else 0
        print(f"{'연료':^12} {sp.total_fuel_kg:>13.2f} kg {op.total_fuel_kg:>13.2f} kg "
              f"{fuel_diff:>+13.2f} kg {fuel_pct:>+9.2f}%")
        
        # 결론
        print("\n" + "─"*70)
        print(f"💡 결론:")
        if abs(fuel_diff) < 0.01:
            print(f"   ⚠️  두 경로가 거의 동일합니다 (날씨 데이터 미반영 가능성)")
        elif fuel_diff < 0:
            print(f"   ✅ 최적 경로가 연료 {abs(fuel_diff):.2f} kg 절감!")
            print(f"   ✅ 연료 효율 개선: {abs(fuel_pct):.2f}%")
        else:
            print(f"   ⚠️  최단 경로가 {fuel_diff:.2f} kg 더 효율적 (알고리즘 재검토 필요)")
    
    print("\n" + "="*70)
    print("✅ 테스트 완료!")
    print("="*70)
    
    return True

# ================================================================
# 5. 실행
# ================================================================

if __name__ == "__main__":
    try:
        # 커맨드라인 인자 파싱
        waypoints, config_args = parse_arguments()
        
        print("="*70)
        print("🚢 Ship Routing Algorithm - Python Integration Test")
        print("="*70)
        print(f"\n【 입력 웨이포인트 】({len(waypoints)}개)")
        for i, (lat, lon) in enumerate(waypoints, 1):
            print(f"  WP{i}: 위도 {lat}, 경도 {lon}")
        
        print(f"\n【 항해 설정 】")
        print(f"  선박 속도: {config_args.speed} m/s")
        print(f"  흘수: {config_args.draft} m")
        print(f"  그리드 크기: {config_args.grid_size} km")
        
        # 환경 설정
        setup_environment()
        
        # 모듈 임포트
        algorithm_module = import_module()
        
        # 테스트 실행
        success = run_test(algorithm_module, waypoints, config_args)
        
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