#pragma once

#include "../data_loading/grid_builder.h"
#include "../data_loading/weather_loader.h"
#include "../route_analysis/waypoint_snapper.h"
#include "../results/route_results.h"
#include "../types/voyage_types.h"
#include "../pathfinding/route_planner.h"
#include <memory>
#include <string>

/**
 * @class ShipRouter
 * @brief 선박 경로 탐색을 위한 메인 API 클래스
 * 
 * 전체 워크플로우:
 * 1. Initialize: GEBCO/GSHHS 데이터 로딩
 * 2. OptimizeVoyage: 웨이포인트 입력 → 최적 경로 반환
 *    - 그리드 생성
 *    - 웨이포인트 스냅핑
 *    - 최단 경로 탐색
 *    - 최적 경로 탐색
 *    - 결과 분석 및 반환
 */
class ShipRouter {
public:
    ShipRouter();
    ~ShipRouter();
    
    // ================================================================
    // 초기화
    // ================================================================
    
    /**
     * @brief 데이터 파일 로딩 (GEBCO, GSHHS)
     * @param gebco_path GEBCO NetCDF 파일 경로
     * @param gshhs_path GSHHS Shapefile 경로
     * @return bool 성공 여부
     */
    bool Initialize(
        const std::string& gebco_path,
        const std::string& gshhs_path
    );
    
    /**
     * @brief 날씨 데이터 로딩 (선택적)
     * @param weather_dir 날씨 데이터 디렉토리 경로
     * @return bool 성공 여부
     */
    bool LoadWeatherData(const std::string& weather_dir);
    
    bool IsInitialized() const { return isInitialized_; }
    
    // ================================================================
    // 메인 API: 경로 계산
    // ================================================================
    
    /**
     * @brief 웨이포인트로부터 최단/최적 경로 계산
     * @param waypoints 사용자 입력 웨이포인트 리스트
     * @param config 항해 설정 (속도, 드래프트, 그리드 크기 등)
     * @return VoyageResult 최단/최적 경로 및 분석 결과
     */
    VoyageResult CalculateRoute(
        const std::vector<GeoCoordinate>& waypoints,
        const VoyageConfig& config = VoyageConfig()
    );
    
    // ================================================================
    // 개별 단계 API (디버깅/테스트용)
    // ================================================================
    
    /**
     * @brief 1단계: 그리드 생성
     */
    NavigableGrid BuildGrid(
        const std::vector<GeoCoordinate>& waypoints,
        double cellSizeKm = 5.0,
        int marginCells = 20
    );
    
    /**
     * @brief 2단계: 웨이포인트 스냅핑
     */
    std::vector<SnappingInfo> SnapWaypoints(
        const NavigableGrid& grid,
        const std::vector<GeoCoordinate>& waypoints,
        double maxRadiusKm = 50.0
    );
    
    /**
     * @brief 3단계: 최단 경로 탐색
     */
    SinglePathResult FindShortestPath(
        const NavigableGrid& grid,
        const std::vector<GeoCoordinate>& snapped_waypoints,
        const VoyageConfig& config
    );
    
    /**
     * @brief 4단계: 최적 경로 탐색 (연료 최적화)
     */
    SinglePathResult FindOptimalPath(
        const NavigableGrid& grid,
        const std::vector<GeoCoordinate>& snapped_waypoints,
        const VoyageConfig& config,
        const std::map<std::string, WeatherDataInput>& weather_data
    );

private:
    // ================================================================
    // 내부 상태
    // ================================================================
    bool isInitialized_;
    
    // 데이터 로더들
    std::unique_ptr<GridBuilder> gridBuilder_;
    std::unique_ptr<WeatherLoader> weatherLoader_;
    
    // 날씨 데이터
    std::map<std::string, WeatherDataInput> weatherData_;
    bool hasWeatherData_;
    
    // ================================================================
    // 내부 헬퍼 함수들
    // ================================================================
    
    /**
     * @brief 여러 웨이포인트를 거치는 경로 탐색
     * @param grid Navigable grid
     * @param waypoints Waypoint list (geo coordinates)
     * @param planner Route planner strategy
     * @param config Voyage configuration
     * @param use_weather Whether to use weather data
     * @return SinglePathResult Complete path result
     */
    SinglePathResult FindPathThroughWaypoints(
        const NavigableGrid& grid,
        const std::vector<GeoCoordinate>& waypoints,
        IRoutePlanner& planner,
        const VoyageConfig& config,
        bool use_weather
    );
    
    /**
     * @brief 경로 분석 수행 (그리드 경로 → PathPointDetail 생성)
     * @param path_grid Grid path
     * @param grid Navigable grid
     * @param config Voyage configuration
     * @param use_weather Whether to use weather data
     * @param total_cost Total cost from A* (distance or fuel)
     * @param total_time_hours Total time from A*
     * @return SinglePathResult with detailed analysis
     */
    SinglePathResult AnalyzePathResult(
        const std::vector<GridCoordinate>& path_grid,
        const NavigableGrid& grid,
        const VoyageConfig& config,
        bool use_weather,
        double total_cost,
        double total_time_hours
    );
    
    /**
     * @brief 에러 결과 생성
     */
    VoyageResult MakeErrorResult(const std::string& error_message);
    SinglePathResult MakeErrorPathResult(const std::string& error_message);
};