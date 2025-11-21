#include "optimized_planner.h"
#include "a_star_engine.h"
#include "path_utils.h"
#include "../utils/geo_calculations.h"
#include "../utils/time_calculator.h"
#include "../utils/fuel_calculator.h"
#include <iostream>

OptimizedRoutePlanner::OptimizedRoutePlanner(
    const NavigableGrid& grid,
    const VoyageInfo& voyageInfo,
    unsigned int startTimeSec,
    const std::map<std::string, WeatherDataInput>& weatherData,
    double shipSpeedMps)
    : grid_(grid)
    , voyageInfoBase_(voyageInfo)
    , startTimeSec_(startTimeSec)
    , weatherData_(weatherData)
    , shipSpeedMps_(shipSpeedMps)
    , minFuelRateKgPerHour_(0.0)
    , goalGeo_(0.0, 0.0)
{
}

void OptimizedRoutePlanner::InitializeHeuristic(
    const GridCoordinate& start,
    const GridCoordinate& goal)
{
    // Store goal for heuristic computation
    goalGeo_ = grid_.GridToGeo(goal);
    
    // Calculate minimum fuel rate at start point (ideal conditions)
    GeoCoordinate startGeo = grid_.GridToGeo(start);
    
    // Calculate heading for heuristic
    VoyageInfo vInfo = voyageInfoBase_;
    vInfo.heading = calculateBearing(startGeo, goalGeo_);
    
    // Calculate ideal fuel rate (zero weather conditions)
    minFuelRateKgPerHour_ = fuelCalculator_zero(
        startTimeSec_,
        startGeo.latitude,
        startGeo.longitude,
        vInfo
    );
}

PathSearchResult OptimizedRoutePlanner::FindPath(
    const NavigableGrid& grid,
    const GridCoordinate& start,
    const GridCoordinate& goal)
{
    std::cout << "[OptimizedPlanner] Finding fuel-optimized path..." << std::endl;
    
    // Initialize heuristic parameters
    InitializeHeuristic(start, goal);
    
    // Run A* search
    PathSearchResult result = AStarEngine::Search(grid, start, goal, *this);
    
    if (result.IsSuccess()) {
        std::cout << "[OptimizedPlanner] ✓ Path found with fuel: " 
                  << result.total_cost << " kg" << std::endl;
    } else {
        std::cerr << "[OptimizedPlanner] ✗ Path not found" << std::endl;
    }
    
    return result;
}

EdgeCostResult OptimizedRoutePlanner::ComputeEdgeCost(
    const GridCoordinate& from,
    const GridCoordinate& to,
    double accumulatedTimeHours) const
{
    // 1) Transform grid -> geo
    GeoCoordinate fromGeo = grid_.GridToGeo(from);
    GeoCoordinate toGeo = grid_.GridToGeo(to);
    
    // 2) Calculate distance [km] and time [hours] between from/to
    double distKm = greatCircleDistance(
        fromGeo.latitude, fromGeo.longitude,
        toGeo.latitude, toGeo.longitude
    );
    
    double timeHours = timeCalculator(distKm, shipSpeedMps_);
    
    // 3) Calculate mid point
    GeoCoordinate midGeo = {
        (fromGeo.latitude + toGeo.latitude) / 2.0,
        (fromGeo.longitude + toGeo.longitude) / 2.0
    };
    
    // 4) Calculate heading
    VoyageInfo vInfo = voyageInfoBase_;
    vInfo.heading = calculateBearing(fromGeo, toGeo);
    
    // 5) Calculate mid time in seconds
    unsigned int midTimeSec = startTimeSec_ 
        + static_cast<unsigned int>(
            (accumulatedTimeHours + timeHours / 2.0) * 3600.0
        );
    
    // 6) Calculate fuel rate [kg/h] at mid point & mid time
    double fuelRateKgPerHour = fuelCalculator(
        midTimeSec,
        midGeo.latitude, midGeo.longitude,
        weatherData_,
        vInfo
    );
    
    // 7) Calculate fuel consumption [kg] for this edge
    double fuelKg = fuelRateKgPerHour * timeHours;
    
    EdgeCostResult result;
    result.cost = fuelKg;
    result.deltaTimeHours = timeHours;
    
    return result;
}

double OptimizedRoutePlanner::ComputeHeuristic(
    const GridCoordinate& current,
    const GridCoordinate& goal) const
{
    // Convert current position to geographic coordinates
    GeoCoordinate currentGeo = grid_.GridToGeo(current);
    
    // Calculate remaining distance to goal
    double distKm = greatCircleDistance(
        currentGeo.latitude, currentGeo.longitude,
        goalGeo_.latitude, goalGeo_.longitude
    );
    
    // Calculate time required
    double timeHours = timeCalculator(distKm, shipSpeedMps_);
    
    // Heuristic = minimum possible fuel (ideal conditions)
    return minFuelRateKgPerHour_ * timeHours;
}

bool OptimizedRoutePlanner::IsValidTransition(
    const PathNode& current_node,
    const GridCoordinate& neighbor_pos) const
{
    // Calculate direction to neighbor
    int dx = neighbor_pos.row - current_node.pos.row;
    int dy = neighbor_pos.col - current_node.pos.col;
    
    // Check angle constraint
    return AngleCheck(current_node, dx, dy);
}