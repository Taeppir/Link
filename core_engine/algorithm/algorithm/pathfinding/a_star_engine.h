#pragma once

#include "path_types.h"
#include "route_planner.h"
#include "../types/grid_types.h"

/**
 * @class AStarEngine
 * @brief Generic A* pathfinding engine
 * 
 * This engine executes A* algorithm using a provided IRoutePlanner strategy.
 * The strategy determines cost calculation, heuristic, and transition rules.
 */
class AStarEngine {
public:
    /**
     * @brief Execute A* search with given strategy
     * 
     * @param grid Navigable grid
     * @param start Start grid coordinate
     * @param goal Goal grid coordinate
     * @param planner Strategy for cost/heuristic computation
     * @return PathSearchResult with path and total cost
     */
    static PathSearchResult Search(
        const NavigableGrid& grid,
        const GridCoordinate& start,
        const GridCoordinate& goal,
        const IRoutePlanner& planner
    );
};