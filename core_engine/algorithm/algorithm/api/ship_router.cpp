#include "ship_router.h"
#include "../route_analysis/waypoint_snapper.h"
#include "../utils/geo_calculations.h"
#include "../utils/time_calculator.h"
#include "../utils/fuel_calculator.h"
#include "../utils/weather_interpolation.h"
#include <iostream>
#include <chrono>

// TODO: pathfinding 엔진 include
// #include "../pathfinding/a_star_pathfinder.h"
// #include "../pathfinding/strategies/shortest_path_strategy.h"
// #include "../pathfinding/strategies/optimized_path_strategy.h"

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
    std::cout << "[ShipRouter] Initializing..." << std::endl;
    
    try {
        // GridBuilder 생성 및 데이터 로딩
        gridBuilder_ = std::make_unique<GridBuilder>();
        
        if (!gridBuilder_->LoadBathymetryData(gebco_path)) {
            std::cerr << "[ERROR] Failed to load GEBCO data" << std::endl;
            return false;
        }
        
        if (!gridBuilder_->LoadCoastlineData(gshhs_path)) {
            std::cerr << "[ERROR] Failed to load GSHHS data" << std::endl;
            return false;
        }
        
        std::cout << "[ShipRouter] ✓ Data loaded successfully" << std::endl;
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
        weatherData_ = weatherLoader_->LoadWeatherData();
        
        hasWeatherData_ = !weatherData_.empty();
        
        if (hasWeatherData_) {
            std::cout << "[ShipRouter] ✓ Weather data loaded ("
                      << weatherData_.size() << " files)" << std::endl;
        } else {
            std::cout << "[ShipRouter] ⚠ No weather data loaded, "
                      << "will use zero weather conditions" << std::endl;
        }
        
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
    
    std::cout << "\n=== ShipRouter: Starting route calculation ===" << std::endl;
    std::cout << "Waypoints: " << waypoints.size() << std::endl;
    std::cout << "Config: cellSize=" << config.gridCellSizeKm 
              << "km, speed=" << config.shipSpeedMps << "m/s" << std::endl;
    
    try {
        // ============================================================
        // STEP 1: 그리드 생성
        // ============================================================
        std::cout << "\n[STEP 1] Building navigable grid..." << std::endl;
        NavigableGrid grid = BuildGrid(
            waypoints,
            config.gridCellSizeKm,
            config.gridMarginCells
        );
        
        // ============================================================
        // STEP 2: 웨이포인트 스냅핑
        // ============================================================
        std::cout << "\n[STEP 2] Snapping waypoints..." << std::endl;
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
        
        // 스냅된 좌표 추출
        std::vector<GeoCoordinate> snapped_waypoints;
        for (const auto& info : snapping_info) {
            snapped_waypoints.push_back(info.snapped);
        }
        
        // ============================================================
        // STEP 3: 최단 경로 탐색
        // ============================================================
        SinglePathResult shortest_result;
        if (config.calculateShortest) {
            std::cout << "\n[STEP 3] Finding shortest path..." << std::endl;
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
            std::cout << "\n[STEP 4] Finding optimal path..." << std::endl;
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
        
        std::cout << "\n=== Route calculation completed in " 
                  << duration.count() << "ms ===" << std::endl;
        
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
    // TODO: A* 최단 경로 알고리즘 호출
    std::cout << "[TODO] Shortest path algorithm not implemented yet" << std::endl;
    
    // 임시 구현 (나중에 실제 A* 알고리즘으로 교체)
    SinglePathResult result;
    result.success = false;
    result.error_message = "Shortest path algorithm not implemented";
    return result;
    
    /*
    // 실제 구현 예시 (pathfinding 엔진 완성 후):
    
    AStarPathfinder pathfinder(grid);
    ShortestPathStrategy strategy;
    
    std::vector<GridCoordinate> path_grid = pathfinder.FindPath(
        snapped_waypoints,
        &strategy
    );
    
    return AnalyzePathResult(path_grid, grid, config, false);
    */
}

SinglePathResult ShipRouter::FindOptimalPath(
    const NavigableGrid& grid,
    const std::vector<GeoCoordinate>& snapped_waypoints,
    const VoyageConfig& config,
    const std::map<std::string, WeatherDataInput>& weather_data)
{
    // TODO: A* 최적 경로 알고리즘 호출
    std::cout << "[TODO] Optimal path algorithm not implemented yet" << std::endl;
    
    // 임시 구현
    SinglePathResult result;
    result.success = false;
    result.error_message = "Optimal path algorithm not implemented";
    return result;
    
    /*
    // 실제 구현 예시:
    
    AStarPathfinder pathfinder(grid);
    OptimizedPathStrategy strategy(config, weather_data);
    
    std::vector<GridCoordinate> path_grid = pathfinder.FindPath(
        snapped_waypoints,
        &strategy
    );
    
    return AnalyzePathResult(path_grid, grid, config, true);
    */
}

// ================================================================
// 내부 헬퍼 함수
// ================================================================

SinglePathResult ShipRouter::AnalyzePathResult(
    const std::vector<GridCoordinate>& path_grid,
    const NavigableGrid& grid,
    const VoyageConfig& config,
    bool use_weather)
{
    // TODO: RouteAnalyzer 사용
    std::cout << "[TODO] Path analysis not implemented yet" << std::endl;
    
    SinglePathResult result;
    result.success = false;
    result.error_message = "Path analysis not implemented";
    return result;
    
    /*
    // 실제 구현 예시:
    
    RouteAnalyzer analyzer;
    
    VoyageInfo voyage_info;
    voyage_info.shipSpeed = config.shipSpeedMps;
    voyage_info.draft = config.draftM;
    voyage_info.trim = config.trimM;
    
    auto weather = use_weather ? weatherData_ : std::map<std::string, WeatherDataInput>();
    
    SingleRouteAnalysis analysis = analyzer.AnalyzePath(
        path_grid,
        grid,
        voyage_info,
        config.startTimeUnix,
        weather
    );
    
    // SingleRouteAnalysis를 SinglePathResult로 변환
    SinglePathResult result;
    result.success = true;
    result.summary.total_distance_km = analysis.summary.total_distance_km;
    result.summary.total_time_hours = analysis.summary.total_time_hours;
    result.summary.total_fuel_kg = analysis.summary.total_fuel_kg;
    result.summary.average_speed_mps = analysis.summary.average_speed_mps;
    result.summary.average_fuel_rate_kg_per_hour = analysis.summary.average_fuel_rate_kg_per_hour;
    
    // path_details 변환
    for (const auto& detail : analysis.point_details) {
        PathPointDetail point;
        point.position = detail.coordinate;
        point.cumulative_time_hours = detail.cumulative_time_hours;
        point.cumulative_distance_km = detail.cumulative_distance_km;
        point.cumulative_fuel_kg = detail.cumulative_fuel_kg;
        point.fuel_rate_kg_per_hour = detail.fuel_rate_kg_per_hour;
        point.speed_mps = detail.current_speed_mps;
        point.weather = detail.weather;
        result.path_details.push_back(point);
    }
    
    return result;
    */
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