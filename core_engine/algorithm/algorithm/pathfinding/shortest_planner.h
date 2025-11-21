#pragma once

#include "route_planner.h"
#include "path_types.h"
#include "../types/grid_types.h"

/**
 * @class ShortestRoutePlanner
 * @brief Distance-based shortest path planner
 * 
 * Uses distance as the cost metric.
 * Cost = Great Circle Distance between grid cells
 * Heuristic = Great Circle Distance to goal
 */
class ShortestRoutePlanner : public IRoutePlanner {
public:
    /**
     * @brief Constructor
     * @param grid Navigable grid reference
     * @param shipSpeedMps Ship speed in meters per second
     */
    ShortestRoutePlanner(
        const NavigableGrid& grid,
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
    double shipSpeedMps_;
};