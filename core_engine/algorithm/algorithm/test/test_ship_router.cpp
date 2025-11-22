// test_ship_router.cpp - 상세 출력 버전 (작업 디렉토리: LINK)

#include "../api/ship_router.h"
#include "../types/geo_types.h"
#include "../types/voyage_types.h"
#include "../results/route_results.h"
#include <iostream>
#include <iomanip>
#include <chrono>

// ================================================================
// Helper Functions
// ================================================================

void PrintPathResult(const std::string& name, const SinglePathResult& result) {
    if (!result.success) {
        std::cout << "\n[" << name << "] FAILED: " << result.error_message << std::endl;
        return;
    }
    
    const auto& s = result.summary;
    
    std::cout << "\n[" << name << "]" << std::endl;
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "  Distance:       " << s.total_distance_km << " km" << std::endl;
    std::cout << "  Time:           " << s.total_time_hours << " hours" << std::endl;
    std::cout << "  Fuel:           " << s.total_fuel_kg << " kg" << std::endl;
    std::cout << "  Avg Speed:      " << s.average_speed_mps << " m/s" << std::endl;
    std::cout << "  Avg Fuel Rate:  " << s.average_fuel_rate_kg_per_hour << " kg/h" << std::endl;
    std::cout << "  Path Points:    " << result.path_details.size() << std::endl;
}

void PrintComparison(const SinglePathResult& shortest, const SinglePathResult& optimized) {
    if (!shortest.success || !optimized.success) {
        return;
    }
    
    std::cout << "\n=== Comparison (Optimized vs Shortest) ===" << std::endl;
    std::cout << std::fixed << std::setprecision(2);
    
    // 거리
    double dist_diff = optimized.summary.total_distance_km - shortest.summary.total_distance_km;
    double dist_diff_pct = (dist_diff / shortest.summary.total_distance_km) * 100.0;
    std::cout << "Distance:  " << dist_diff 
              << " km (" << (dist_diff >= 0 ? "+" : "") << dist_diff_pct << "%)" << std::endl;
    
    // 시간
    double time_diff = optimized.summary.total_time_hours - shortest.summary.total_time_hours;
    double time_diff_pct = (time_diff / shortest.summary.total_time_hours) * 100.0;
    std::cout << "Time:      " << time_diff 
              << " h (" << (time_diff >= 0 ? "+" : "") << time_diff_pct << "%)" << std::endl;
    
    // 연료
    double fuel_diff = optimized.summary.total_fuel_kg - shortest.summary.total_fuel_kg;
    double fuel_diff_pct = (fuel_diff / shortest.summary.total_fuel_kg) * 100.0;
    std::cout << "Fuel:      " << fuel_diff 
              << " kg (" << (fuel_diff >= 0 ? "+" : "") << fuel_diff_pct << "%)" << std::endl;
    
    // 요약
    std::cout << "\nSummary: ";
    if (fuel_diff < 0) {
        std::cout << "Saves " << std::abs(fuel_diff) << " kg fuel ("
                  << std::abs(fuel_diff_pct) << "%)" << std::endl;
    } else {
        std::cout << "Uses more fuel" << std::endl;
    }
}

// ================================================================
// Main Test
// ================================================================

int main() {
    try {
        std::cout << "=== ShipRouter Integration Test ===" << std::endl;
        
        // ========================================
        // 1. Initialization
        // ========================================
        std::cout << "\nInitializing..." << std::endl;
        
        ShipRouter router;
        
        // ✨ 작업 디렉토리가 LINK로 설정되어 있으므로 data/ 상대 경로 사용
        if (!router.Initialize(
            "data/gebco/GEBCO_2024_sub_ice_topo.nc",
            "data/gshhs/GSHHS_i_L1.shp")) {
            std::cerr << "ERROR: Initialization failed" << std::endl;
            return 1;
        }
        
        router.LoadWeatherData("data/weather");
        
        // ========================================
        // 2. Route Calculation
        // ========================================
        std::vector<GeoCoordinate> waypoints = {
            {35.0994, 129.0336},  // Busan
            {33.4996, 126.5312},  // Jeju
        };
        
        VoyageConfig config;
        config.shipSpeedMps = 8.0;
        config.draftM = 10.0;
        config.gridCellSizeKm = 5.0;
        config.gridMarginCells = 20;
        config.maxSnapRadiusKm = 50.0;
        config.startTimeUnix = 1577836800;  // 2020-01-01
        config.calculateShortest = true;
        config.calculateOptimized = true;
        
        auto start = std::chrono::high_resolution_clock::now();
        VoyageResult result = router.CalculateRoute(waypoints, config);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        if (!result.success) {
            std::cerr << "\nERROR: Calculation failed - " << result.error_message << std::endl;
            return 1;
        }
        
        std::cout << "OK: Calculation completed (" << duration.count() << " ms)" << std::endl;
        
        // ========================================
        // 3. Results
        // ========================================
        PrintPathResult("Shortest Path", result.shortest_path);
        PrintPathResult("Optimized Path", result.optimized_path);
        PrintComparison(result.shortest_path, result.optimized_path);
        
        std::cout << "\n=== Test PASSED ===" << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "\nERROR: Exception - " << e.what() << std::endl;
        return 1;
    }
}