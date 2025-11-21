//#include "gdal_priv.h"
//#include "../include/ship_router.h"
//#include "../include/short_path.h"
//#include "../include/time_calculator.h"
//#include "../include/geo_utils.h"
//#include "../include/fuel_consumption_calculator.h"
//#include <iostream>
//
//ShipRouter::ShipRouter() : m_is_initialized(false) {}
//
//bool ShipRouter::initialize(const std::string& gebco_path, const std::string& gshhs_path) {
//    try {
//        GDALDriverManager* dm = GetGDALDriverManager();
//        dm->AutoLoadDrivers();
//
//        if (!m_fetcher.LoadCoastlineData(gshhs_path)) {
//            std::cerr << "Failed to load GSHHS data." << std::endl;
//            return false;
//        }
//
//        if (!m_fetcher.LoadBathymetryData(gebco_path)) {
//            std::cerr << "Failed to load GEBCO data." << std::endl;
//            return false;
//        }
//
//        m_weather_data = storeWeatherData();
//        if (m_weather_data.empty()) {
//            std::cout << "Warning: Weather data is empty. Fuel calc will use 0 values." << std::endl;
//        }
//    }
//    catch (const std::exception& e) {
//        std::cerr << "Initialization failed: " << e.what() << std::endl;
//        return false;
//    }
//
//    m_is_initialized = true;
//    return true;
//}
//
//RouteAnalysisResult ShipRouter::calculateShortestPathAnalysis(
//    const std::vector<GeoCoordinate>& waypoints_geo,
//    double ship_speed_mps)
//{
//    RouteAnalysisResult result;
//    if (!m_is_initialized) {
//        result.error_message = "Router is not initialized. Call initialize() first.";
//        return result;
//    }
//    if (waypoints_geo.size() < 2) {
//        result.error_message = "At least 2 waypoints are required.";
//        return result;
//    }
//
//    try {
//        // STEP 3: Create grid
//        std::cout << "Building Navigable Grid..." << std::endl;
//        NavigableGrid navGrid = m_fetcher.BuildNavigableGrid(waypoints_geo, 5, 20);
//
//        // STEP 4: Waypoints convert
//        std::vector<Point> waypoints_grid;
//        for (const auto& geo_wp : waypoints_geo) {
//            waypoints_grid.push_back(navGrid.GeoToGrid(geo_wp));
//        }
//
//        // STEP 5: A* path search
//        auto full_route_result = findRouteWithWaypoints(navGrid, waypoints_grid);
//        FullRouteResults all_segments = full_route_result.first;
//        double astar_total_distance = full_route_result.second;
//
//        // STEP 5.5: Geo path generate
//        for (size_t leg_idx = 0; leg_idx < all_segments.size(); ++leg_idx) {
//            const auto& current_leg_path_grid = all_segments[leg_idx].first;
//            for (size_t i = 0; i < current_leg_path_grid.size(); ++i) {
//                if (leg_idx > 0 && i == 0) continue;
//                result.full_path_geo.push_back(navGrid.GridToGeo(current_leg_path_grid[i]));
//            }
//        }
//
//        // STEP 6: Voyage Analysis
//        VoyageInfo voyageInfo;
//        voyageInfo.shipSpeed = ship_speed_mps;
//        voyageInfo.draft = 10.0;
//        voyageInfo.trim = 0.0;
//        unsigned int simulation_time_sec = m_weather_data.begin()->second.iStartTime;
//
//        double total_time_hours = 0.0;
//        double total_fuel_kg = 0.0;
//        double total_distance_km_calc = 0.0;
//        double total_distance_km_acc = 0.0;
//
//        for (size_t leg_idx = 0; leg_idx < all_segments.size(); ++leg_idx) {
//            const auto& current_leg_path_grid = all_segments[leg_idx].first;
//            size_t leg_total_segments = current_leg_path_grid.size() - 1;
//
//            for (size_t i = 0; i < leg_total_segments; ++i) {
//                GeoCoordinate p1_geo = navGrid.GridToGeo(current_leg_path_grid[i]);
//                GeoCoordinate p2_geo = navGrid.GridToGeo(current_leg_path_grid[i + 1]);
//
//                double segment_dist_km = greatCircleDistance(p1_geo.latitude, p1_geo.longitude, p2_geo.latitude, p2_geo.longitude);
//                double segment_time_hours = timeCalculator(segment_dist_km, ship_speed_mps);
//                voyageInfo.heading = calculateBearing(p1_geo, p2_geo);
//
//                unsigned int current_sim_time_sec = simulation_time_sec + static_cast<unsigned int>(total_time_hours * 3600.0);
//
//                double fuel_rate_kg_per_hour = fuelCalculator(
//                    current_sim_time_sec, p1_geo.latitude, p1_geo.longitude,
//                    m_weather_data, voyageInfo
//                );
//                double segment_fuel_kg = fuel_rate_kg_per_hour * segment_time_hours;
//
//                total_distance_km_acc += segment_dist_km;
//                total_time_hours += segment_time_hours;
//                total_fuel_kg += segment_fuel_kg;
//            }
//        }
//        
//        result.total_distance_km = total_distance_km_acc;
//        result.total_time_hours = total_time_hours;
//        result.total_fuel_kg = total_fuel_kg;
//
//        // TODO: Add optimal path value,SHOULD BE MOVED.
//        result.optimal_distance_km = 0;
//        result.optimal_time_hours = 0;
//        result.optimal_fuel_kg = 0;
//
//        result.success = true;
//
//    }
//    catch (const std::exception& e) {
//        std::cerr << "Calculation failed: " << e.what() << std::endl;
//        result.error_message = e.what();
//        result.success = false;
//    }
//
//    return result;
//}