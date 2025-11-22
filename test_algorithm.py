import os
import sys

# 1. DLL 경로 설정 (기존 유지)
try:
    python_base_dir = sys.base_prefix 
    project_root = os.getcwd()
    lib_dir = os.path.join(project_root, "Lib")
    VCPKG_BIN = os.getenv("VCPKG_BIN", r"C:\vcpkg\installed\x64-windows\bin")

    if sys.version_info >= (3, 8) and os.name == 'nt':
        os.add_dll_directory(python_base_dir)
        if os.path.exists(lib_dir):
            os.add_dll_directory(lib_dir)
        if os.path.exists(VCPKG_BIN):
            os.add_dll_directory(VCPKG_BIN)

    import algorithm_module
    print(f"✅ 모듈 임포트 성공: {algorithm_module}")

except ImportError as e:
    print(f"❌ 모듈 임포트 실패: {e}")
    sys.exit(1)
except Exception as e:
    print(f"⚠️ 설정 오류: {e}")

def run_test():
    router = algorithm_module.ShipRouter()
    
    # [수정됨] 데이터 파일 경로 설정 (이미지 구조 반영)
    # test_algorithm.py 위치(루트) 기준으로 내부 깊숙한 data 폴더를 찾습니다.
    script_dir = os.path.dirname(os.path.abspath(__file__))
    
    # 이미지에 나온 구조: core_engine/algorithm/algorithm/data/gebco...
    # 경로를 유연하게 찾기 위해 리스트로 후보군 설정
    base_data_dir_candidates = [
        os.path.join(script_dir, "core_engine", "algorithm", "algorithm", "data"),  # 원본 소스 위치
        os.path.join(script_dir, "data"),  # (혹시 복사해뒀다면) 루트 data 폴더
    ]
    
    gebco_path = ""
    gshhs_path = ""
    
    for base_dir in base_data_dir_candidates:
        temp_gebco = os.path.join(base_dir, "gebco", "GEBCO_2024_sub_ice_topo.nc")
        temp_gshhs = os.path.join(base_dir, "gshhs", "GSHHS_i_L1.shp")
        
        if os.path.exists(temp_gebco) and os.path.exists(temp_gshhs):
            gebco_path = temp_gebco
            gshhs_path = temp_gshhs
            print(f"✅ 데이터 폴더 발견: {base_dir}")
            break
    
    print("\n[1] 엔진 초기화 중... (데이터 로딩)")
    print(f" - GEBCO: {gebco_path}")
    print(f" - GSHHS: {gshhs_path}")

    if not gebco_path or not os.path.exists(gebco_path):
        print(f"❌ GEBCO 파일을 찾을 수 없습니다. core_engine/algorithm/algorithm/data/gebco 폴더를 확인하세요.")
        return
    if not gshhs_path or not os.path.exists(gshhs_path):
        print(f"❌ GSHHS 파일을 찾을 수 없습니다. core_engine/algorithm/algorithm/data/gshhs 폴더를 확인하세요.")
        return

    # 초기화 실행
    success = router.initialize(gebco_path, gshhs_path)
    
    if not success:
        print("❌ 엔진 초기화 함수가 false를 반환했습니다.")
        return
    print("✅ 엔진 초기화 완료")

    # 3. 테스트 웨이포인트 (부산 -> 싱가포르)
    start_wp = algorithm_module.GeoCoordinate(35.10, 129.04)
    end_wp = algorithm_module.GeoCoordinate(1.29, 103.85)
    
    waypoints = [start_wp, end_wp]
    print(f"\n[2] 경로 탐색 시작: 부산 -> 싱가포르")

    # 4. 경로 계산
    result = router.calculate_route(waypoints)

    if result.success:
        print("\n✅ 경로 탐색 성공!")
        opt = result.optimized_path
        print(f" - 총 거리: {opt.summary.total_distance_km:.2f} km")
        print(f" - 총 연료: {opt.summary.total_fuel_kg:.2f} kg")
        print(f" - 웨이포인트 개수: {len(opt.path_details)}")
    else:
        print(f"\n❌ 경로 탐색 실패: {result.error_message}")
        for info in result.snapping_info:
            if not info.status == algorithm_module.SnappingStatus.SNAPPED and \
               not info.status == algorithm_module.SnappingStatus.ALREADY_NAVIGABLE:
                print(f" - WP Snapping Failed: {info.failure_reason}")

if __name__ == "__main__":
    run_test()