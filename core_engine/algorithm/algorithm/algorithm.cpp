// #include <iostream>
// #include <algorithm>
// #include <fstream>
// #include <vector>
// #include <chrono>
// #include "include/fetch_data.h"
// #include "include/read_weather_data.h"
// #include "include/fuel_consumption_calculator.h"
// #include "include/navigable_grid.h"
// #include "include/short_path.h"
// #include "include/time_calculator.h"
// #include "include/geo_utils.h"
// #include "include/waypoint_snapper.h"
// #include "include/less_fuel.h"
// #include "src/common_path_utils/JSON_maker.h"

// int main() {
//     try {
//         // ===========================================================================
//         // STEP 1: Load Geographic Data (GEBCO + GSHHS)
//         // ===========================================================================
//         FetchData fetcher;
//         if (!fetcher.LoadBathymetryData("data/GEBCO_2024_sub_ice_topo.nc")) {
//             throw std::runtime_error("Failed to load GEBCO data.");
//         }
//         if (!fetcher.LoadCoastlineData("data/GSHHS_i_L1.shp")) {
//             throw std::runtime_error("Failed to load GSHHS data.");
//         }

//         // ===========================================================================
//         // STEP 2: Define Route Waypoints for Path Plaaning
//         // - TODO: Replace with user input
//         // - 빠른 테스트를 위해 부산항-제주도 항로만 설정
//         // ===========================================================================
//         std::vector<GeoCoordinate> all_waypoints_geo = {
//             // WP0: 출발 - 부산항 (항만 내부)
//             {35.0994, 129.0336},

//             // WP1: 경유 - 제주도 제주시청 (육지)
//             {33.4996, 126.5312},

//             //// WP2: 경유 - 일본 오키나와 나하시청 (육지)
//             //{26.2124, 127.6809},

//             //// WP3: 경유 - 대만 가오슝시청 (육지)
//             //{22.6273, 120.3014},

//             ////WP4: 도착 - 싱가포르 (시청/다운타운, 육지)
//             //{1.2903, 103.8520}

//             // 부산 -> 싱가포르 직항
//             // 주의) cell size [km]= 5 로 하면 해상도 부족으로 경로 탐색 실패할 수 있음 -> 1km 권장(그러나 시간 더 걸림)
//             // cell size는 line 60에서 BuildNavigableGrid 호출 시 조정 가능
//             //{35.14, 129.06},{1.24, 103.83}

//         };

//         // ===========================================================================
//         // STEP 3: Build Navigable Grid Map
//         // - Create ROI containing all waypoints
//         // - Parameters: (waypoints, cell size [km], margin [pixel])
//         // ===========================================================================
//         std::cout << "\n=== Building Navigable Grid ==================================\n";
//         NavigableGrid navGrid = fetcher.BuildNavigableGrid(all_waypoints_geo, 5, 20);

//         // ==================================================================
//         // STEP 3.5: Snap Waypoints to Navigable Waters
//         // ==================================================================
//         std::cout << "\n=== Snapping Waypoints to Navigable Waters =======\n";

//         WaypointSnapper wpSnapper(navGrid);

//         auto snap_results = wpSnapper.SnapMultipleWaypoints(
//             all_waypoints_geo,
//             50.0  // 최대 50km 반경
//         );

//         // 보정된 좌표로 교체
//         all_waypoints_geo = wpSnapper.GetSnappedWaypoints(snap_results);

//         // ===========================================================================
//         // STEP 4: Convert Geo Waypoints to Grid Coordinates
//         // - TODO: Move this conversion inside findRouteWithWaypoints()
//         // ===========================================================================
//         std::vector<Point> all_waypoints_grid;
//         std::cout << "\n=== Converting Waypoints to Grid Coordinates ================" << std::endl;
//         for (const auto& geo_wp : all_waypoints_geo) {
//             all_waypoints_grid.push_back(navGrid.GeoToGrid(geo_wp));
//             std::cout << "Geo(" << geo_wp.latitude << ", " << geo_wp.longitude << ") -> Grid("
//                 << all_waypoints_grid.back().row << ", " << all_waypoints_grid.back().col << ")" << std::endl;
//         }

//         // ===========================================================================
//         // STEP 5: Voyage Analysis Setup
//         // ===========================================================================
//         std::cout << "\n=== Initializing Voyage Analysis ==========" << std::endl;

//         std::map<std::string, WeatherDataInput> all_weather_data = storeWeatherData();
//         if (all_weather_data.empty()) {
//             std::cout << "Warning: Weather data is empty. Fuel calculation will use 0 values." << std::endl;
//         }

//         const double SHIP_SPEED_MPS = 8.0;
//         VoyageInfo voyageInfo;
//         voyageInfo.shipSpeed = SHIP_SPEED_MPS;
//         voyageInfo.draft = 10.0;
//         voyageInfo.trim = 0.0;

//         unsigned int simulation_time_sec = 0;
//         if (!all_weather_data.empty()) {
//             simulation_time_sec = all_weather_data.begin()->second.iStartTime;
//         }

//         // ===========================================================================
//         // STEP 6-A: Execute Shortest Path Planning (Distance-based A*)
//         // ===========================================================================
//         std::cout << "\n=== [1] Executing SHORTEST PATH Analysis ===================" << std::endl;

//         auto start_time_shortest = std::chrono::high_resolution_clock::now();

//         auto shortest_route_result = findRouteWithWaypoints(navGrid, all_waypoints_grid);

//         auto end_time_shortest = std::chrono::high_resolution_clock::now();
//         auto duration_shortest = std::chrono::duration_cast<std::chrono::milliseconds>(end_time_shortest - start_time_shortest);

//         FullRouteResults shortest_segments = shortest_route_result.first;
//         double shortest_astar_fuel = shortest_route_result.second;

//         std::vector<GeoCoordinate> short_coordinates;

//         // Analyze shortest path
//         double shortest_distance_km = 0.0;
//         double shortest_time_hours = 0.0;
//         double shortest_fuel_kg = 0.0;

//         for (size_t leg_idx = 0; leg_idx < shortest_segments.size(); ++leg_idx) {
//             const auto& current_leg_path_grid = shortest_segments[leg_idx].first;

//             if (current_leg_path_grid.empty()) continue;

//             size_t leg_total_segments = current_leg_path_grid.size() - 1;

//             for (size_t i = 0; i < leg_total_segments; ++i) {
//                 Point p1_grid = current_leg_path_grid[i];
//                 Point p2_grid = current_leg_path_grid[i + 1];

//                 GeoCoordinate p1_geo = navGrid.GridToGeo(p1_grid);
//                 GeoCoordinate p2_geo = navGrid.GridToGeo(p2_grid);
//                 GeoCoordinate mid_geo = {
//                     (p1_geo.latitude + p2_geo.latitude) / 2.0,
//                      (p1_geo.longitude + p2_geo.longitude) / 2.0
//                 };

//                 short_coordinates.push_back(p1_geo);
//                 short_coordinates.push_back(p2_geo);

//                 double segment_dist_km = greatCircleDistance(p1_geo.latitude, p1_geo.longitude, p2_geo.latitude, p2_geo.longitude);
//                 double segment_time_hours = timeCalculator(segment_dist_km, SHIP_SPEED_MPS);
//                 voyageInfo.heading = calculateBearing(p1_geo, p2_geo);

//                 unsigned int mid_sim_time_sec = simulation_time_sec +
//                     static_cast<unsigned int>((shortest_time_hours + segment_time_hours / 2.0) * 3600.0);

//                 double fuel_rate_kg_per_hour = fuelCalculator(
//                     mid_sim_time_sec,
//                     mid_geo.latitude, mid_geo.longitude,
//                     all_weather_data, voyageInfo
//                 );

//                 double segment_fuel_kg = fuel_rate_kg_per_hour * segment_time_hours;

//                 shortest_distance_km += segment_dist_km;
//                 shortest_time_hours += segment_time_hours;
//                 shortest_fuel_kg += segment_fuel_kg;
//             }
//         }
        
//         SaveGeoPointsToJson(short_coordinates,
//             "short_path.json",
//             "#808080");


//         std::cout << "\n[SHORTEST PATH RESULTS]" << std::endl;
//         std::cout.precision(2);
//         std::cout << std::fixed;
//         std::cout << "  - Total Distance: " << shortest_distance_km << " km" << std::endl;
//         std::cout << "  - Total Time:     " << shortest_time_hours << " hours" << std::endl;
//         std::cout << "  - Total Fuel:     " << shortest_fuel_kg << " kg"  << std::endl;
//         std::cout << "  - Total Search Time: " << duration_shortest.count() << " ms" << std::endl;

//         // ===========================================================================
//         // STEP 6-B: Execute Optimized Path Planning (Fuel-optimized A*)
//         // ===========================================================================
//         std::cout << "\n=== [2] Executing OPTIMIZED PATH Analysis ==================" << std::endl;

//         auto start_time_optimized = std::chrono::high_resolution_clock::now();

//         auto optimized_route_result = findRouteWithWaypoints_optimized(navGrid, all_waypoints_grid, voyageInfo, simulation_time_sec, all_weather_data);

//         auto end_time_optimized = std::chrono::high_resolution_clock::now();
//         auto duration_optimized = std::chrono::duration_cast<std::chrono::milliseconds>(end_time_optimized - start_time_optimized);

//         FullRouteResults optimized_segments = optimized_route_result.first;
//         double optimized_astar_fuel = optimized_route_result.second;

//         std::vector<GeoCoordinate> optimized_coordinates;


//         // Analyze optimized path
//         double optimized_distance_km = 0.0;
//         double optimized_time_hours = 0.0;
//         double optimized_fuel_kg = 0.0;

//         for (size_t leg_idx = 0; leg_idx < optimized_segments.size(); ++leg_idx) {
//             const auto& current_leg_path_grid = optimized_segments[leg_idx].first;

//             if (current_leg_path_grid.empty()) continue;

//             size_t leg_total_segments = current_leg_path_grid.size() - 1;

//             for (size_t i = 0; i < leg_total_segments; ++i) {
//                 Point p1_grid = current_leg_path_grid[i];
//                 Point p2_grid = current_leg_path_grid[i + 1];

//                 GeoCoordinate p1_geo = navGrid.GridToGeo(p1_grid);
//                 GeoCoordinate p2_geo = navGrid.GridToGeo(p2_grid);
//                 GeoCoordinate mid_geo = {
//                     (p1_geo.latitude + p2_geo.latitude) / 2.0,
//                      (p1_geo.longitude + p2_geo.longitude) / 2.0
//                 };

//                 optimized_coordinates.push_back(p1_geo);
//                 optimized_coordinates.push_back(p2_geo);

//                 double segment_dist_km = greatCircleDistance(p1_geo.latitude, p1_geo.longitude, p2_geo.latitude, p2_geo.longitude);
//                 double segment_time_hours = timeCalculator(segment_dist_km, SHIP_SPEED_MPS);
//                 voyageInfo.heading = calculateBearing(p1_geo, p2_geo);

//                 unsigned int mid_sim_time_sec = simulation_time_sec +
//                     static_cast<unsigned int>((optimized_time_hours + segment_time_hours / 2.0) * 3600.0);

//                 double fuel_rate_kg_per_hour = fuelCalculator(
//                     mid_sim_time_sec,
//                     mid_geo.latitude, mid_geo.longitude,
//                     all_weather_data, voyageInfo
//                 );

//                 double segment_fuel_kg = fuel_rate_kg_per_hour * segment_time_hours;

//                 optimized_distance_km += segment_dist_km;
//                 optimized_time_hours += segment_time_hours;
//                 optimized_fuel_kg += segment_fuel_kg;
//             }
//         }

//         SaveGeoPointsToJson(optimized_coordinates,
//             "optimized_path.json",
//             "#FF0000");

//         std::cout << "\n[OPTIMIZED PATH RESULTS]" << std::endl;
//         std::cout.precision(2);
//         std::cout << std::fixed;
//         std::cout << "  - Total Distance: " << optimized_distance_km << " km" << std::endl;
//         std::cout << "  - Total Time:     " << optimized_time_hours << " hours" << std::endl;
//         std::cout << "  - Total Fuel:     " << optimized_fuel_kg << " kg"  << std::endl;
//         std::cout << "  - Total Search Time: " << duration_optimized.count() << " ms" << std::endl;

//         // ===========================================================================
//         // STEP 7: Compare Results
//         // ===========================================================================
//         std::cout << "\n=== COMPARISON: Shortest vs Optimized ======================" << std::endl;
//         std::cout.precision(2);
//         std::cout << std::fixed;

//         double distance_diff_km = optimized_distance_km - shortest_distance_km;
//         double distance_diff_pct = (distance_diff_km / shortest_distance_km) * 100.0;

//         double time_diff_hours = optimized_time_hours - shortest_time_hours;
//         double time_diff_pct = (time_diff_hours / shortest_time_hours) * 100.0;

//         double fuel_diff_kg = optimized_fuel_kg - shortest_fuel_kg;
//         double fuel_diff_pct = (fuel_diff_kg / shortest_fuel_kg) * 100.0;

//         std::cout << "\nDistance:" << std::endl;
//         std::cout << "  Shortest:  " << shortest_distance_km << " km" << std::endl;
//         std::cout << "  Optimized: " << optimized_distance_km << " km" << std::endl;
//         std::cout << "  Difference: " << (distance_diff_km >= 0 ? "+" : "") << distance_diff_km
//             << " km (" << (distance_diff_pct >= 0 ? "+" : "") << distance_diff_pct << "%)" << std::endl;

//         std::cout << "\nTime:" << std::endl;
//         std::cout << "  Shortest:  " << shortest_time_hours << " hours" << std::endl;
//         std::cout << "  Optimized: " << optimized_time_hours << " hours" << std::endl;
//         std::cout << "  Difference: " << (time_diff_hours >= 0 ? "+" : "") << time_diff_hours
//             << " hours (" << (time_diff_pct >= 0 ? "+" : "") << time_diff_pct << "%)" << std::endl;

//         std::cout << "\nFuel Consumption:" << std::endl;
//         std::cout << "  Shortest:  " << shortest_fuel_kg << " kg (" << (shortest_fuel_kg / 1000.0) << " ton)" << std::endl;
//         std::cout << "  Optimized: " << optimized_fuel_kg << " kg" << std::endl;
//         std::cout << "  Difference: " << (fuel_diff_kg >= 0 ? "+" : "") << fuel_diff_kg
//             << " kg (" << (fuel_diff_pct >= 0 ? "+" : "") << fuel_diff_pct << "%)" << std::endl;

//         std::cout << "\n=============================================================" << std::endl;

//     }
//     catch (const std::exception& e) {
//         std::cerr << "\nAn exception occurred: " << e.what() << std::endl;
//         return -1;
//     }
//     return 0;
// }
