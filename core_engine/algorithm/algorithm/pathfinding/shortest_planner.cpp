#include "shortest_planner.h"
#include "a_star_engine.h"
#include "path_utils.h"
#include "../utils/geo_calculations.h"
#include "../utils/time_calculator.h"
#include <iostream>

ShortestRoutePlanner::ShortestRoutePlanner(
    const NavigableGrid& grid,
    double shipSpeedMps)
    : grid_(grid)
    , shipSpeedMps_(shipSpeedMps)
{
}

PathSearchResult ShortestRoutePlanner::FindPath(
    const NavigableGrid& grid,
    const GridCoordinate& start,
    const GridCoordinate& goal)
{
    std::cout << "[ShortestPlanner] Finding shortest path..." << std::endl;
    
    PathSearchResult result = AStarEngine::Search(grid, start, goal, *this);
    
    if (result.IsSuccess()) {
        std::cout << "[ShortestPlanner] ✓ Path found with distance: " 
                  << result.total_cost << " km" << std::endl;
    } else {
        std::cerr << "[ShortestPlanner] ✗ Path not found" << std::endl;
    }
    
    return result;
}

EdgeCostResult ShortestRoutePlanner::ComputeEdgeCost(
    const GridCoordinate& from,
    const GridCoordinate& to,
    double /*accumulatedTimeHours*/) const
{
    // Convert grid coordinates to geographic coordinates
    GeoCoordinate fromGeo = grid_.GridToGeo(from);
    GeoCoordinate toGeo = grid_.GridToGeo(to);
    
    // Calculate great circle distance
    double distKm = greatCircleDistance(
        fromGeo.latitude, fromGeo.longitude,
        toGeo.latitude, toGeo.longitude
    );
    
    // Calculate time to traverse
    double timeHours = timeCalculator(distKm, shipSpeedMps_);
    
    EdgeCostResult result;
    result.cost = distKm;
    result.deltaTimeHours = timeHours;
    
    return result;
}

double ShortestRoutePlanner::ComputeHeuristic(
    const GridCoordinate& current,
    const GridCoordinate& goal) const
{
    // Convert to geographic coordinates
    GeoCoordinate currentGeo = grid_.GridToGeo(current);
    GeoCoordinate goalGeo = grid_.GridToGeo(goal);
    
    // Heuristic = straight-line distance to goal
    double distKm = greatCircleDistance(
        currentGeo.latitude, currentGeo.longitude,
        goalGeo.latitude, goalGeo.longitude
    );
    
    return distKm;
}

bool ShortestRoutePlanner::IsValidTransition(
    const PathNode& current_node,
    const GridCoordinate& neighbor_pos) const
{
    // Calculate direction to neighbor
    int dx = neighbor_pos.row - current_node.pos.row;
    int dy = neighbor_pos.col - current_node.pos.col;
    
    // Check angle constraint
    return AngleCheck(current_node, dx, dy);
}