#pragma once

#include "path_types.h"
#include "../types/grid_types.h"
#include "../types/geo_types.h"

/**
 * @interface IRoutePlanner
 * @brief Strategy interface for different route planning algorithms
 * 
 * This interface allows different planning strategies:
 * - Shortest path (distance-based)
 * - Optimized path (fuel-based with weather)
 * - Future: Speed-optimized, etc.
 */
class IRoutePlanner {
public:
    virtual ~IRoutePlanner() = default;
    
    // ================================================================
    // Core Planning Method
    // ================================================================
    
    /**
     * @brief Find path from start to goal
     * 
     * @param grid Navigable grid
     * @param start Start grid coordinate
     * @param goal Goal grid coordinate
     * @return PathSearchResult with path and cost
     */
    virtual PathSearchResult FindPath(
        const NavigableGrid& grid,
        const GridCoordinate& start,
        const GridCoordinate& goal
    ) = 0;
    
    // ================================================================
    // Strategy-Specific Methods (for A* algorithm)
    // ================================================================
    
    /**
     * @brief Compute edge traversal cost
     * 
     * @param from Start grid coordinate
     * @param to End grid coordinate
     * @param accumulatedTimeHours Time accumulated so far
     * @return EdgeCostResult with cost and time delta
     */
    virtual EdgeCostResult ComputeEdgeCost(
        const GridCoordinate& from,
        const GridCoordinate& to,
        double accumulatedTimeHours
    ) const = 0;
    
    /**
     * @brief Compute heuristic estimate to goal
     * 
     * @param current Current grid coordinate
     * @param goal Goal grid coordinate
     * @return Estimated cost to goal
     */
    virtual double ComputeHeuristic(
        const GridCoordinate& current,
        const GridCoordinate& goal
    ) const = 0;
    
    /**
     * @brief Check if transition from->to is valid (angle check, etc.)
     * 
     * @param current_node Current node with parent info
     * @param neighbor_pos Neighbor position to check
     * @return true if transition is valid
     */
    virtual bool IsValidTransition(
        const PathNode& current_node,
        const GridCoordinate& neighbor_pos
    ) const = 0;
};