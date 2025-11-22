#pragma once

#include "route_planner.h"
#include "path_types.h"
#include "../types/grid_types.h"
#include "../types/voyage_types.h"
#include "../types/weather_types.h"
#include <map>
#include <string>

/**
 * @class OptimizedRoutePlanner
 * @brief Fuel-optimized path planner with weather consideration
 * 
 * Uses fuel consumption as the cost metric.
 * Cost = Fuel consumption calculated from weather conditions
 * Heuristic = Minimum possible fuel consumption (ideal conditions)
 */
class OptimizedRoutePlanner : public IRoutePlanner {
public:
    /**
     * @brief Constructor
     * @param grid Navigable grid reference
     * @param voyageInfo Base voyage information (draft, trim, speed)
     * @param startTimeSec Simulation start time in seconds
     * @param weatherData Weather data map
     * @param shipSpeedMps Ship speed in meters per second
     */
    OptimizedRoutePlanner(
        const NavigableGrid& grid,
        const VoyageInfo& voyageInfo,
        unsigned int startTimeSec,
        const std::map<std::string, WeatherDataInput>& weatherData,
        double shipSpeedMps
    );
    
    // ================================================================
    // IRoutePlanner Interface Implementation
    // ================================================================
    
    PathSearchResult FindPath(
        const NavigableGrid& grid,
        const GridCoordinate& start,
        const GridCoordinate& goal
    ) override;
    
    EdgeCostResult ComputeEdgeCost(
        const GridCoordinate& from,
        const GridCoordinate& to,
        double accumulatedTimeHours
    ) const override;
    
    double ComputeHeuristic(
        const GridCoordinate& current,
        const GridCoordinate& goal
    ) const override;
    
    bool IsValidTransition(
        const PathNode& current_node,
        const GridCoordinate& neighbor_pos
    ) const override;

private:
    const NavigableGrid& grid_;
    VoyageInfo voyageInfoBase_;
    unsigned int startTimeSec_;
    const std::map<std::string, WeatherDataInput>& weatherData_;
    double shipSpeedMps_;
    
    // Heuristic parameters
    double minFuelRateKgPerHour_;
    GeoCoordinate goalGeo_;
    
    /**
     * @brief Initialize minimum fuel rate for heuristic
     */
    void InitializeHeuristic(const GridCoordinate& start, const GridCoordinate& goal);
};