#include "ship_router.h"
#include "../route_analysis/waypoint_snapper.h"
#include "../pathfinding/shortest_planner.h"
#include "../pathfinding/optimized_planner.h"
#include "../utils/geo_calculations.h"
#include "../utils/time_calculator.h"
#include "../utils/fuel_calculator.h"
#include "../utils/weather_interpolation.h"
#include <iostream>
#include <chrono>

ShipRouter::ShipRouter()
    : isInitialized_(false)
    , hasWeatherData_(false)
{
}

ShipRouter::~ShipRouter() {
}

// ================================================================
// 초기화 구현
// ================================================================

bool ShipRouter::Initialize(
    const std::string& gebco_path,
    const std::string& gshhs_path)
{
    std::cout << "Initializing ShipRouter..." << std::endl;

    try {
        gridBuilder_ = std::make_unique<GridBuilder>();
        
        if (!gridBuilder_->LoadBathymetryData(gebco_path)) {
            std::cerr << "[ERROR] Failed to load GEBCO data" << std::endl;
            return false;
        }
        
        if (!gridBuilder_->LoadCoastlineData(gshhs_path)) {
            std::cerr << "[ERROR] Failed to load GSHHS data" << std::endl;
            return false;
        }
        
        std::cout << "[ShipRouter] Data loaded successfully" << std::endl;
        isInitialized_ = true;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Initialization failed: " << e.what() << std::endl;
        return false;
    }
}

bool ShipRouter::LoadWeatherData(const std::string& weather_dir) {
    std::cout << "[ShipRouter] Loading weather data..." << std::endl;
    
    try {
        weatherLoader_ = std::make_unique<WeatherLoader>();
        weatherData_ = weatherLoader_->LoadWeatherData(weather_dir);
        hasWeatherData_ = !weatherData_.empty();
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Weather data loading failed: " << e.what() << std::endl;
        hasWeatherData_ = false;
        return false;
    }
}

// ================================================================
// 메인 API: CalculateRoute
// ================================================================

VoyageResult ShipRouter::CalculateRoute(
    const std::vector<GeoCoordinate>& waypoints,
    const VoyageConfig& config)
{
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // 초기화 체크
    if (!isInitialized_) {
        return MakeErrorResult("ShipRouter not initialized");
    }
    
    // 웨이포인트 개수 체크
    if (waypoints.size() < 2) {
        return MakeErrorResult("At least 2 waypoints required");
    }
    
    std::cout << "\n[ShipRouter] Route Calculation -------------------------------" << std::endl;
    std::cout << "Waypoints: " << waypoints.size() << ", Grid: " << config.gridCellSizeKm << "km" << std::endl;
    
    try {
        // ============================================================
        // STEP 1: 그리드 생성
        // ============================================================
        // std::cout << "\n(1) Building grid..." << std::endl;
        NavigableGrid grid = BuildGrid(
            waypoints,
            config.gridCellSizeKm,
            config.gridMarginCells
        );
        
        // ============================================================
        // STEP 2: 웨이포인트 스냅핑
        // ============================================================
        // std::cout << "\n(2) Snapping waypoints..." << std::endl;
        std::vector<SnappingInfo> snapping_info = SnapWaypoints(
            grid,
            waypoints,
            config.maxSnapRadiusKm
        );
        
        // 스냅핑 실패 체크
        bool all_snapped = true;
        for (const auto& info : snapping_info) {
            if (!info.IsSuccess()) {
                all_snapped = false;
                std::cerr << "[ERROR] Waypoint snapping failed: "
                          << info.failure_reason << std::endl;
            }
        }
        
        if (!all_snapped) {
            VoyageResult result = MakeErrorResult("Waypoint snapping failed");
            result.snapping_info = snapping_info;
            return result;
        }
        
        std::vector<GeoCoordinate> snapped_waypoints;
        for (const auto& info : snapping_info) {
            snapped_waypoints.push_back(info.snapped);
        }
        
        // ============================================================
        // STEP 3: 최단 경로 탐색
        // ============================================================
        SinglePathResult shortest_result;
        if (config.calculateShortest) {
            // std::cout << "\n(3) Finding shortest path..." << std::endl;
            shortest_result = FindShortestPath(grid, snapped_waypoints, config);
            
            if (!shortest_result.success) {
                VoyageResult result = MakeErrorResult("Shortest path finding failed: " + shortest_result.error_message);
                result.snapping_info = snapping_info;
                return result;
            }
        }
        
        // ============================================================
        // STEP 4: 최적 경로 탐색 (연료 최적화)
        // ============================================================
        SinglePathResult optimal_result;
        if (config.calculateOptimized) {
            // std::cout << "\n(4) Finding optimized path..." << std::endl;
            optimal_result = FindOptimalPath(
                grid,
                snapped_waypoints,
                config,
                weatherData_
            );
            
            if (!optimal_result.success) {
                VoyageResult result = MakeErrorResult("Optimal path finding failed: " + optimal_result.error_message);
                result.snapping_info = snapping_info;
                result.shortest_path = shortest_result;
                return result;
            }
        }
        
        // ============================================================
        // STEP 5: 결과 조합
        // ============================================================
        VoyageResult result;
        result.success = true;
        result.snapping_info = snapping_info;
        result.shortest_path = shortest_result;
        result.optimized_path = optimal_result;
        
        // 계산 시간
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        // std::cout << "\n=== Route calculation completed in " << duration.count() << "ms ===" << std::endl;
        
        return result;
        
    } catch (const std::exception& e) {
        return MakeErrorResult(std::string("Exception: ") + e.what());
    }
}

// ================================================================
// 개별 단계 구현
// ================================================================

NavigableGrid ShipRouter::BuildGrid(
    const std::vector<GeoCoordinate>& waypoints,
    double cellSizeKm,
    int marginCells)
{
    return gridBuilder_->BuildNavigableGrid(
        waypoints,
        cellSizeKm,
        marginCells
    );
}

std::vector<SnappingInfo> ShipRouter::SnapWaypoints(
    const NavigableGrid& grid,
    const std::vector<GeoCoordinate>& waypoints,
    double maxRadiusKm)
{
    WaypointSnapper snapper(grid, nullptr);  // TODO: port snapper 추가?
    return snapper.SnapMultipleWaypoints(waypoints, maxRadiusKm);
}

SinglePathResult ShipRouter::FindShortestPath(
    const NavigableGrid& grid,
    const std::vector<GeoCoordinate>& snapped_waypoints,
    const VoyageConfig& config)
{
    // Create shortest path planner
    ShortestRoutePlanner planner(grid, config.shipSpeedMps);
    
    // Find path through all waypoints
    return FindPathThroughWaypoints(
        grid,
        snapped_waypoints,
        planner,
        config,
        false  // no weather
    );
}

SinglePathResult ShipRouter::FindOptimalPath(
    const NavigableGrid& grid,
    const std::vector<GeoCoordinate>& snapped_waypoints,
    const VoyageConfig& config,
    const std::map<std::string, WeatherDataInput>& weather_data)
{
    // Prepare voyage info
    VoyageInfo voyageInfo;
    voyageInfo.shipSpeed = config.shipSpeedMps;
    voyageInfo.draft = config.draftM;
    voyageInfo.trim = config.trimM;
    
    // Create optimized path planner
    OptimizedRoutePlanner planner(
        grid,
        voyageInfo,
        config.startTimeUnix,
        weather_data,
        config.shipSpeedMps
    );
    
    // Find path through all waypoints
    return FindPathThroughWaypoints(
        grid,
        snapped_waypoints,
        planner,
        config,
        true  // use weather
    );
}

// ================================================================
// 내부 헬퍼 함수
// ================================================================

SinglePathResult ShipRouter::FindPathThroughWaypoints(
    const NavigableGrid& grid,
    const std::vector<GeoCoordinate>& waypoints,
    IRoutePlanner& planner,
    const VoyageConfig& config,
    bool use_weather)
{
    if (waypoints.size() < 2) {
        return MakeErrorPathResult("At least 2 waypoints required");
    }
    
    // Containers for complete path
    std::vector<GridCoordinate> complete_path;
    double total_cost = 0.0;
    double total_time_hours = 0.0;
    
    // Process each segment
    for (size_t i = 0; i < waypoints.size() - 1; ++i) {
        // Convert waypoints to grid coordinates
        GridCoordinate start = grid.GeoToGrid(waypoints[i]);
        GridCoordinate goal = grid.GeoToGrid(waypoints[i + 1]);
        
        // Find path for this segment
        PathSearchResult segment_result = planner.FindPath(grid, start, goal);
        
        // Check if path found
        if (!segment_result.IsSuccess()) {
            // std::cerr << "[ERROR] Path not found for segment " << (i + 1) << std::endl;
            return MakeErrorPathResult("Path not found for segment " + std::to_string(i + 1));
        }
        
        // Accumulate cost and time
        total_cost += segment_result.total_cost;
        total_time_hours += segment_result.total_time_hours;
        
        // Append path
        if (i == 0) {
            complete_path.insert(
                complete_path.end(),
                segment_result.path.begin(),
                segment_result.path.end()
            );
        } else {
            complete_path.insert(
                complete_path.end(),
                segment_result.path.begin() + 1,
                segment_result.path.end()
            );
        }
        
    }
    
    // Analyze path and create detailed result
    return AnalyzePathResult(
        complete_path,
        grid,
        config,
        use_weather,
        total_cost,
        total_time_hours
    );
}

SinglePathResult ShipRouter::AnalyzePathResult(
    const std::vector<GridCoordinate>& path_grid,
    const NavigableGrid& grid,
    const VoyageConfig& config,
    bool use_weather,
    double total_cost,
    double total_time_hours)
{
    SinglePathResult result;
    result.success = true;
    
    // Set summary
    result.summary.total_time_hours = total_time_hours;
    
    if (use_weather) {
        // For optimized path: cost is fuel
        result.summary.total_fuel_kg = total_cost;
        result.summary.total_distance_km = 0.0;  // Calculate from path

        

    } else {
        // For shortest path: cost is distance
        result.summary.total_distance_km = total_cost;
        result.summary.total_fuel_kg = 0.0;  // Calculate from path
    }
    
    // Prepare voyage info for detailed analysis
    VoyageInfo voyageInfo;
    voyageInfo.shipSpeed = config.shipSpeedMps;
    voyageInfo.draft = config.draftM;
    voyageInfo.trim = config.trimM;
    
    // Generate detailed point-by-point data
    double cumulative_time = 0.0;
    double cumulative_distance = 0.0;
    double cumulative_fuel = 0.0;
    
    for (size_t i = 0; i < path_grid.size(); ++i) {
        PathPointDetail point;
        
        // Position
        point.position = grid.GridToGeo(path_grid[i]);
        
        // For points after the first, calculate segment data
        if (i > 0) {
            GeoCoordinate prevGeo = grid.GridToGeo(path_grid[i - 1]);
            GeoCoordinate currGeo = point.position;
            
            // Calculate segment distance
            double segment_dist = greatCircleDistance(
                prevGeo.latitude, prevGeo.longitude,
                currGeo.latitude, currGeo.longitude
            );
            
            // Calculate segment time
            double segment_time = timeCalculator(segment_dist, config.shipSpeedMps);
            
            // Calculate heading
            voyageInfo.heading = calculateBearing(prevGeo, currGeo);
            
            // Mid-point for weather/fuel calculation
            GeoCoordinate midGeo = {
                (prevGeo.latitude + currGeo.latitude) / 2.0,
                (prevGeo.longitude + currGeo.longitude) / 2.0
            };
            
            unsigned int mid_time_sec = config.startTimeUnix +
                static_cast<unsigned int>((cumulative_time + segment_time / 2.0) * 3600.0);
            
            // Get weather at mid-point
            if (use_weather && !weatherData_.empty()) {
                point.weather = getWeatherAtCoordinate(
                    weatherData_,
                    mid_time_sec,
                    midGeo.latitude,
                    midGeo.longitude
                );
                
                // Calculate fuel rate
                point.fuel_rate_kg_per_hour = fuelCalculator(
                    mid_time_sec,
                    midGeo.latitude, midGeo.longitude,
                    weatherData_,
                    voyageInfo
                );
            } else {
                // Zero weather
                point.fuel_rate_kg_per_hour = fuelCalculator_zero(
                    mid_time_sec,
                    midGeo.latitude, midGeo.longitude,
                    voyageInfo
                );
            }
            
            // Calculate segment fuel
            double segment_fuel = point.fuel_rate_kg_per_hour * segment_time;
            
            // Update cumulatives
            cumulative_time += segment_time;
            cumulative_distance += segment_dist;
            cumulative_fuel += segment_fuel;
            
            // Store segment data
            point.speed_mps = config.shipSpeedMps;
            point.heading_degrees = voyageInfo.heading;
        }
        
        // Store cumulative values
        point.cumulative_time_hours = cumulative_time;
        point.cumulative_distance_km = cumulative_distance;
        point.cumulative_fuel_kg = cumulative_fuel;
        
        result.path_details.push_back(point);
    }
    
    // Update summary with calculated values
    if (!use_weather) {
        // For shortest path, we calculated fuel
        result.summary.total_fuel_kg = cumulative_fuel;
    } else {
        // For optimized path, we calculated distance
        result.summary.total_distance_km = cumulative_distance;
    }
    
    // Calculate averages
    if (total_time_hours > 0.0) {
        result.summary.average_speed_mps = 
            (cumulative_distance * 1000.0) / (total_time_hours * 3600.0);
        result.summary.average_fuel_rate_kg_per_hour = 
            cumulative_fuel / total_time_hours;
    }
    
    return result;
}

VoyageResult ShipRouter::MakeErrorResult(const std::string& error_message) {
    VoyageResult result;
    result.success = false;
    result.error_message = error_message;
    return result;
}

SinglePathResult ShipRouter::MakeErrorPathResult(const std::string& error_message) {
    SinglePathResult result;
    result.success = false;
    result.error_message = error_message;
    return result;
}