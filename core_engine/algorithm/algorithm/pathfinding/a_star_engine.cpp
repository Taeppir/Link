#include "a_star_engine.h"
#include "path_utils.h"
#include <queue>
#include <map>
#include <algorithm>
#include <limits>
#include <iostream>

PathSearchResult AStarEngine::Search(
    const NavigableGrid& grid,
    const GridCoordinate& start,
    const GridCoordinate& goal,
    const IRoutePlanner& planner)
{
    // ================================================================
    // 1. Validate start and goal
    // ================================================================
    if (!IsValidAndNavigable(grid, start) || !IsValidAndNavigable(grid, goal)) {
        std::cerr << "[AStarEngine] Error: Start or Goal position is not navigable." << std::endl;
        return PathSearchResult();
    }
    
    if (start == goal) {
        PathSearchResult result;
        result.path = { start };
        result.total_cost = 0.0;
        result.total_time_hours = 0.0;
        return result;
    }
    
    // ================================================================
    // 2. Initialize A* data structures
    // ================================================================
    std::map<GridCoordinate, double> g_scores;
    g_scores[start] = 0.0;
    
    std::map<GridCoordinate, GridCoordinate> parents;
    std::map<GridCoordinate, bool> closed_list;
    std::priority_queue<PathNode, std::vector<PathNode>, ComparePathNode> open_list;
    
    // ================================================================
    // 3. Initialize start node
    // ================================================================
    double initial_h = planner.ComputeHeuristic(start, goal);
    PathNode start_node(start, 0.0, initial_h, GridCoordinate(-1, -1), 0.0);
    open_list.push(start_node);
    
    // ================================================================
    // 4. A* main loop
    // ================================================================
    while (!open_list.empty()) {
        PathNode current = open_list.top();
        open_list.pop();
        
        GridCoordinate current_pos = current.pos;
        
        // Skip if already processed
        if (closed_list.count(current_pos)) {
            continue;
        }
        
        // Check if goal reached
        if (current_pos == goal) {
            // Reconstruct path
            std::vector<GridCoordinate> path;
            GridCoordinate p = goal;
            
            while (parents.count(p)) {
                path.push_back(p);
                p = parents[p];
            }
            path.push_back(start);
            std::reverse(path.begin(), path.end());
            
            // Create result
            PathSearchResult result;
            result.path = path;
            result.total_cost = current.g_cost;
            result.total_time_hours = current.accumulated_time_hours;
            
            return result;
        }
        
        // Mark as closed
        closed_list[current_pos] = true;
        
        double accumulated_time_hours = current.accumulated_time_hours;
        
        // ================================================================
        // 5. Expand neighbors (8 directions)
        // ================================================================
        for (int i = 0; i < 8; ++i) {
            int new_row = current_pos.row + DX_8DIR[i];
            int new_col = current_pos.col + DY_8DIR[i];
            GridCoordinate neighbor_pos(new_row, new_col);
            
            // Check basic validity
            if (!IsValidAndNavigable(grid, neighbor_pos)) {
                continue;
            }
            
            // Skip if already processed
            if (closed_list.count(neighbor_pos)) {
                continue;
            }
            
            // Check transition validity (angle check, etc.)
            if (!planner.IsValidTransition(current, neighbor_pos)) {
                continue;
            }
            
            // Compute edge cost
            EdgeCostResult edge = planner.ComputeEdgeCost(
                current_pos,
                neighbor_pos,
                accumulated_time_hours
            );
            
            double travel_cost = edge.cost;
            double time_hours_delta = edge.deltaTimeHours;
            double new_g_cost = current.g_cost + travel_cost;
            
            // Check if this is a better path
            double current_best_g = g_scores.count(neighbor_pos) 
                ? g_scores[neighbor_pos] 
                : std::numeric_limits<double>::infinity();
            
            if (new_g_cost < current_best_g) {
                // Update g_score and parent
                g_scores[neighbor_pos] = new_g_cost;
                parents[neighbor_pos] = current_pos;
                
                // Compute heuristic
                double h_cost = planner.ComputeHeuristic(neighbor_pos, goal);
                
                // Compute accumulated time
                double neighbor_time_hours = accumulated_time_hours + time_hours_delta;
                
                // Create and push neighbor node
                PathNode neighbor_node(
                    neighbor_pos,
                    new_g_cost,
                    h_cost,
                    current_pos,
                    neighbor_time_hours
                );
                
                open_list.push(neighbor_node);
            }
        }
    }
    
    // ================================================================
    // 6. Path not found
    // ================================================================
    std::cerr << "[AStarEngine] Error: Path not found from (" 
              << start.row << ", " << start.col << ") to (" 
              << goal.row << ", " << goal.col << ")" << std::endl;
    
    return PathSearchResult();
}