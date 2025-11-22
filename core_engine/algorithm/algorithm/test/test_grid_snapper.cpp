// #include "data_loading/grid_builder.h"
// #include "route_analysis/waypoint_snapper.h"
// #include "results/route_results.h"
// #include "types/geo_types.h"
// #include <iostream>
// #include <fstream>

// int main() {
//     try {


//         std::cout << "=== Grid Builder & Waypoint Snapper Test ===" << std::endl;

//         // ========================================
//         // Part 1: GridBuilder 테스트
//         // ========================================
//         std::cout << "\n[Part 1] Grid Builder Test" << std::endl;
//         std::cout << "==========================================" << std::endl;

//         // GridBuilder 생성
//         GridBuilder builder;

//         // ✨ CMake가 작업 디렉토리를 CMAKE_SOURCE_DIR로 설정하므로 ./data/ 사용
//         std::string gebcoPath = "../data/gebco/GEBCO_2024_sub_ice_topo.nc";
//         std::string gshhsPath = "../data/gshhs/GSHHS_i_L1.shp";
    
//         std::cout << "Loading GEBCO from: \"" << gebcoPath << "\"" << std::endl;
//         if (builder.LoadBathymetryData(gebcoPath)) {
//             std::cout << "✓ GEBCO loaded successfully" << std::endl;
//         } else {
//             std::cerr << "✗ Failed to load GEBCO" << std::endl;
//             std::cerr << "Tip: Ensure data file exists at: " << gebcoPath << std::endl;
//             return 1;
//         }

//         std::cout << "Loading GSHHS from: \"" << gshhsPath << "\"" << std::endl;
//         if (builder.LoadCoastlineData(gshhsPath)) {
//             std::cout << "✓ GSHHS loaded successfully" << std::endl;
//         } else {
//             std::cerr << "✗ Failed to load GSHHS" << std::endl;
//             std::cerr << "Tip: Ensure data file exists at: " << gshhsPath << std::endl;
//             return 1;
//         }

//         // 테스트 웨이포인트 (부산 -> 제주)
//         std::vector<GeoCoordinate> waypoints = {
//             {35.0994, 129.0336},  // 부산
//             {33.4996, 126.5312}   // 제주
//         };

//         std::cout << "\n=== Building Grid ===" << std::endl;

//         // 그리드 생성 (Debug 모드면 자동으로 로그 출력)
//         NavigableGrid grid = builder.BuildNavigableGrid(waypoints, 5.0, 5);

//         std::cout << "\n✓ Grid Built Successfully" << std::endl;
//         std::cout << "Grid size: " << grid.Rows() << " x " << grid.Cols() << std::endl;

//         // ========================================
//         // Part 2: WaypointSnapper 테스트
//         // ========================================
//         std::cout << "\n\n[Part 2] Waypoint Snapper Test" << std::endl;
//         std::cout << "==========================================" << std::endl;

//         // WaypointSnapper 생성 (port snapper 없이)
//         WaypointSnapper snapper(grid, nullptr);

//         // 스냅핑 테스트 (육지 근처 좌표 포함)
//         std::vector<GeoCoordinate> testPoints = {
//             {35.0994, 129.0336},  // 부산 (육지)
//             {35.1500, 129.1000},  // 부산 앞바다
//             {33.4996, 126.5312},  // 제주 (육지)
//             {34.0000, 127.5000}   // 중간 해역
//         };

//         std::cout << "\nTesting " << testPoints.size() << " waypoints..." << std::endl;

//         // 스냅핑 수행
//         std::vector<SnappingInfo> results = snapper.SnapMultipleWaypoints(
//             testPoints,
//             50.0  // 50km search radius
//         );

//         // 결과 출력
//         std::cout << "\n=== Snapping Results ===" << std::endl;
//         for (size_t i = 0; i < results.size(); ++i) {
//             const auto& info = results[i];
            
//             std::cout << "\nWaypoint " << i << ":" << std::endl;
//             std::cout << "  Original: (" << info.original.latitude 
//                     << ", " << info.original.longitude << ")" << std::endl;
            
//             switch (info.status) {
//             case SnappingStatus::ALREADY_NAVIGABLE:
//                 std::cout << "  Already navigable" << std::endl;
//                 break;
//             case SnappingStatus::SNAPPED:
//                 std::cout << "  Snapped: (" << info.snapped.latitude 
//                         << ", " << info.snapped.longitude << ")" << std::endl;
//                 std::cout << "  Distance: " << info.snapping_distance_km << " km" << std::endl;
//                 break;
//             case SnappingStatus::FAILED:
//                 std::cout << "  Failed: " << info.failure_reason << std::endl;
//                 break;
//             }
//         }

//         // 성공한 웨이포인트 개수 확인
//         std::vector<GeoCoordinate> snappedCoords = snapper.GetSnappedWaypoints(results);
//         std::cout << "\n✓ Successfully snapped " << snappedCoords.size() 
//                   << " out of " << testPoints.size() << " waypoints." << std::endl;

//         // ========================================
//         // Test Summary
//         // ========================================
//         std::cout << "\n\n==========================================" << std::endl;
//         std::cout << "=== All Tests Completed Successfully ===" << std::endl;
//         std::cout << "==========================================" << std::endl;

//         return 0;
//     }
//     catch (const std::exception& e) {
//         std::cerr << "\n✗ Test failed: " << e.what() << std::endl;
//         return 1;
//     }
// }