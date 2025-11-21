#include "api/ship_router.h"
#include "types/geo_types.h"
#include "types/voyage_types.h"
#include "results/route_results.h"
#include <iostream>
#include <iomanip>
#include <chrono>

// ================================================================
// Helper Functions
// ================================================================

void PrintHeader(const std::string& title) {
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << title << std::endl;
    std::cout << std::string(60, '=') << std::endl;
}

void PrintSnappingResults(const std::vector<SnappingInfo>& snapping_info) {
    PrintHeader("Waypoint Snapping Results");
    
    for (size_t i = 0; i < snapping_info.size(); ++i) {
        const auto& info = snapping_info[i];
        
        std::cout << "\n[Waypoint " << (i + 1) << "]" << std::endl;
        std::cout << "  Original: (" << std::fixed << std::setprecision(4)
                  << info.original.latitude << ", " 
                  << info.original.longitude << ")" << std::endl;
        
        switch (info.status) {
        case SnappingStatus::ALREADY_NAVIGABLE:
            std::cout << "  Status: ✓ Already navigable" << std::endl;
            break;
        case SnappingStatus::SNAPPED:
            std::cout << "  Status: ✓ Snapped to navigable water" << std::endl;
            std::cout << "  Snapped: (" << info.snapped.latitude 
                      << ", " << info.snapped.longitude << ")" << std::endl;
            std::cout << "  Distance: " << info.snapping_distance_km << " km" << std::endl;
            break;
        case SnappingStatus::FAILED:
            std::cout << "  Status: ✗ FAILED" << std::endl;
            std::cout << "  Reason: " << info.failure_reason << std::endl;
            break;
        }
    }
}

void PrintPathSummary(const std::string& path_type, const SinglePathResult& result) {
    PrintHeader(path_type + " - Summary");
    
    if (!result.success) {
        std::cout << "✗ Path not found" << std::endl;
        std::cout << "Error: " << result.error_message << std::endl;
        return;
    }
    
    const auto& summary = result.summary;
    
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "\n📊 Route Statistics:" << std::endl;
    std::cout << "  • Total Distance:    " << summary.total_distance_km << " km" << std::endl;
    std::cout << "  • Total Time:        " << summary.total_time_hours << " hours" << std::endl;
    std::cout << "  • Total Fuel:        " << summary.total_fuel_kg << " kg (" 
              << summary.total_fuel_kg / 1000.0 << " tons)" << std::endl;
    std::cout << "  • Average Speed:     " << summary.average_speed_mps << " m/s" << std::endl;
    std::cout << "  • Average Fuel Rate: " << summary.average_fuel_rate_kg_per_hour 
              << " kg/h" << std::endl;
    std::cout << "  • Path Points:       " << result.path_details.size() << std::endl;
}

void PrintPathDetails(const std::string& path_type, const SinglePathResult& result, int max_points = 10) {
    if (!result.success || result.path_details.empty()) {
        return;
    }
    
    PrintHeader(path_type + " - Detailed Path (First " + std::to_string(max_points) + " points)");
    
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "\n";
    std::cout << std::setw(5) << "No." 
              << std::setw(12) << "Latitude"
              << std::setw(12) << "Longitude"
              << std::setw(12) << "Dist(km)"
              << std::setw(12) << "Time(h)"
              << std::setw(12) << "Fuel(kg)"
              << std::setw(12) << "Speed(m/s)"
              << std::endl;
    std::cout << std::string(80, '-') << std::endl;
    
    size_t num_to_print = std::min(static_cast<size_t>(max_points), result.path_details.size());
    
    for (size_t i = 0; i < num_to_print; ++i) {
        const auto& point = result.path_details[i];
        
        std::cout << std::setw(5) << (i + 1)
                  << std::setw(12) << point.position.latitude
                  << std::setw(12) << point.position.longitude
                  << std::setw(12) << std::setprecision(2) << point.cumulative_distance_km
                  << std::setw(12) << std::setprecision(2) << point.cumulative_time_hours
                  << std::setw(12) << std::setprecision(1) << point.cumulative_fuel_kg
                  << std::setw(12) << std::setprecision(2) << point.speed_mps
                  << std::endl;
    }
    
    if (result.path_details.size() > num_to_print) {
        std::cout << "  ... (" << (result.path_details.size() - num_to_print) 
                  << " more points)" << std::endl;
    }
}

void CompareResults(const SinglePathResult& shortest, const SinglePathResult& optimized) {
    if (!shortest.success || !optimized.success) {
        std::cout << "\n⚠ Cannot compare: one or both paths failed" << std::endl;
        return;
    }
    
    PrintHeader("Comparison: Shortest vs Optimized");
    
    std::cout << std::fixed << std::setprecision(2);
    
    // Distance comparison
    double dist_diff = optimized.summary.total_distance_km - shortest.summary.total_distance_km;
    double dist_diff_pct = (dist_diff / shortest.summary.total_distance_km) * 100.0;
    
    std::cout << "\n📏 Distance:" << std::endl;
    std::cout << "  Shortest:  " << shortest.summary.total_distance_km << " km" << std::endl;
    std::cout << "  Optimized: " << optimized.summary.total_distance_km << " km" << std::endl;
    std::cout << "  Difference: " << (dist_diff >= 0 ? "+" : "") << dist_diff 
              << " km (" << (dist_diff_pct >= 0 ? "+" : "") << dist_diff_pct << "%)" << std::endl;
    
    // Time comparison
    double time_diff = optimized.summary.total_time_hours - shortest.summary.total_time_hours;
    double time_diff_pct = (time_diff / shortest.summary.total_time_hours) * 100.0;
    
    std::cout << "\n⏱️ Time:" << std::endl;
    std::cout << "  Shortest:  " << shortest.summary.total_time_hours << " hours" << std::endl;
    std::cout << "  Optimized: " << optimized.summary.total_time_hours << " hours" << std::endl;
    std::cout << "  Difference: " << (time_diff >= 0 ? "+" : "") << time_diff 
              << " hours (" << (time_diff_pct >= 0 ? "+" : "") << time_diff_pct << "%)" << std::endl;
    
    // Fuel comparison
    double fuel_diff = optimized.summary.total_fuel_kg - shortest.summary.total_fuel_kg;
    double fuel_diff_pct = (fuel_diff / shortest.summary.total_fuel_kg) * 100.0;
    
    std::cout << "\n⛽ Fuel Consumption:" << std::endl;
    std::cout << "  Shortest:  " << shortest.summary.total_fuel_kg << " kg ("
              << shortest.summary.total_fuel_kg / 1000.0 << " tons)" << std::endl;
    std::cout << "  Optimized: " << optimized.summary.total_fuel_kg << " kg ("
              << optimized.summary.total_fuel_kg / 1000.0 << " tons)" << std::endl;
    std::cout << "  Difference: " << (fuel_diff >= 0 ? "+" : "") << fuel_diff 
              << " kg (" << (fuel_diff_pct >= 0 ? "+" : "") << fuel_diff_pct << "%)" << std::endl;
    
    // Summary
    std::cout << "\n💡 Summary:" << std::endl;
    if (fuel_diff < 0) {
        std::cout << "  ✓ Optimized path saves " << std::abs(fuel_diff) << " kg ("
                  << std::abs(fuel_diff) / 1000.0 << " tons) of fuel!" << std::endl;
        if (dist_diff > 0) {
            std::cout << "  → At the cost of " << dist_diff << " km (" 
                      << dist_diff_pct << "%) extra distance" << std::endl;
        }
    } else {
        std::cout << "  ⚠ Optimized path uses MORE fuel (may need algorithm tuning)" << std::endl;
    }
}

// ================================================================
// Main Test
// ================================================================

int main() {
    try {
        PrintHeader("ShipRouter Full Integration Test");
        
        // ========================================
        // Step 1: Initialize ShipRouter
        // ========================================
        std::cout << "\n[Step 1] Initializing ShipRouter..." << std::endl;
        
        ShipRouter router;
        
        // GEBCO data
        std::string gebcoPath = "C:/Users/kth12/Downloads/link/core_engine/algorithm/algorithm/data/GEBCO_2024_sub_ice_topo.nc";
        std::cout << "  Loading GEBCO: " << gebcoPath << std::endl;
        
        std::string gshhsPath = "C:/Users/kth12/Downloads/link/core_engine/algorithm/algorithm/data/GSHHS_i_L1.shp";
        std::cout << "  Loading GSHHS: " << gshhsPath << std::endl;
        

        if (!router.Initialize(gebcoPath, gshhsPath)) {
            std::cerr << "✗ Failed to initialize ShipRouter" << std::endl;
            return 1;
        }
        
        std::cout << "  ✓ ShipRouter initialized successfully" << std::endl;
        
        // Load weather data (optional)
        std::cout << "\n  Loading weather data..." << std::endl;
        if (router.LoadWeatherData("weather/")) {
            std::cout << "  ✓ Weather data loaded" << std::endl;
        } else {
            std::cout << "  ⚠ No weather data (will use zero conditions)" << std::endl;
        }
        
        // ========================================
        // Step 2: Define Waypoints
        // ========================================
        PrintHeader("Step 2: Waypoint Configuration");
        
        // Test waypoints (Busan -> Jeju -> Okinawa)
        std::vector<GeoCoordinate> waypoints = {
            {35.0994, 129.0336},  // Busan (port)
            {33.4996, 126.5312},  // Jeju (port)
            //{26.2124, 127.6809}   // Okinawa (optional - uncomment for longer route)
        };
        
        std::cout << "\nConfigured " << waypoints.size() << " waypoints:" << std::endl;
        for (size_t i = 0; i < waypoints.size(); ++i) {
            std::cout << "  " << (i + 1) << ". (" 
                      << std::fixed << std::setprecision(4)
                      << waypoints[i].latitude << ", " 
                      << waypoints[i].longitude << ")" << std::endl;
        }
        
        // ========================================
        // Step 3: Configure Voyage Parameters
        // ========================================
        PrintHeader("Step 3: Voyage Configuration");
        
        VoyageConfig config;
        config.shipSpeedMps = 8.0;              // 8 m/s (~15.5 knots)
        config.draftM = 10.0;                   // 10 meters draft
        config.trimM = 0.0;                     // Level trim
        config.gridCellSizeKm = 5.0;            // 5 km grid resolution
        config.gridMarginCells = 20;            // 20 cell margin
        config.maxSnapRadiusKm = 50.0;          // 50 km snap radius
        config.startTimeUnix = 0;               // Simulation start time
        config.calculateShortest = true;        // Calculate shortest path
        config.calculateOptimized = true;       // Calculate optimized path
        
        std::cout << "\nVoyage Parameters:" << std::endl;
        std::cout << "  • Ship Speed:      " << config.shipSpeedMps << " m/s" << std::endl;
        std::cout << "  • Draft:           " << config.draftM << " m" << std::endl;
        std::cout << "  • Grid Cell Size:  " << config.gridCellSizeKm << " km" << std::endl;
        std::cout << "  • Snap Radius:     " << config.maxSnapRadiusKm << " km" << std::endl;
        std::cout << "  • Calculate Both:  " 
                  << (config.calculateShortest && config.calculateOptimized ? "Yes" : "No") 
                  << std::endl;
        
        // ========================================
        // Step 4: Calculate Routes
        // ========================================
        PrintHeader("Step 4: Route Calculation");
        
        std::cout << "\nStarting route calculation..." << std::endl;
        auto start_time = std::chrono::high_resolution_clock::now();
        
        VoyageResult result = router.CalculateRoute(waypoints, config);
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        std::cout << "\n✓ Calculation completed in " << duration.count() << " ms" << std::endl;
        
        // ========================================
        // Step 5: Check Results
        // ========================================
        if (!result.success) {
            std::cerr << "\n✗ Route calculation failed!" << std::endl;
            std::cerr << "Error: " << result.error_message << std::endl;
            return 1;
        }
        
        std::cout << "\n✓ Route calculation succeeded!" << std::endl;
        
        // ========================================
        // Step 6: Display Results
        // ========================================
        
        // Snapping results
        PrintSnappingResults(result.snapping_info);
        
        // Shortest path
        if (config.calculateShortest) {
            PrintPathSummary("SHORTEST PATH", result.shortest_path);
            PrintPathDetails("SHORTEST PATH", result.shortest_path, 15);
        }
        
        // Optimized path
        if (config.calculateOptimized) {
            PrintPathSummary("OPTIMIZED PATH", result.optimized_path);
            PrintPathDetails("OPTIMIZED PATH", result.optimized_path, 15);
        }
        
        // Comparison
        if (config.calculateShortest && config.calculateOptimized) {
            CompareResults(result.shortest_path, result.optimized_path);
        }
        
        // ========================================
        // Final Summary
        // ========================================
        PrintHeader("Test Summary");
        
        std::cout << "\n✅ All tests completed successfully!" << std::endl;
        std::cout << "\nResults:" << std::endl;
        std::cout << "  • Waypoints processed: " << waypoints.size() << std::endl;
        std::cout << "  • Waypoints snapped:   " << result.snapping_info.size() << std::endl;
        
        if (result.shortest_path.success) {
            std::cout << "  • Shortest path:       ✓ Found (" 
                      << result.shortest_path.path_details.size() << " points)" << std::endl;
        }
        
        if (result.optimized_path.success) {
            std::cout << "  • Optimized path:      ✓ Found (" 
                      << result.optimized_path.path_details.size() << " points)" << std::endl;
        }
        
        std::cout << "  • Total time:          " << duration.count() << " ms" << std::endl;
        
        std::cout << "\n" << std::string(60, '=') << std::endl;
        std::cout << "🎉 ShipRouter Integration Test PASSED!" << std::endl;
        std::cout << std::string(60, '=') << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "\n✗ Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}