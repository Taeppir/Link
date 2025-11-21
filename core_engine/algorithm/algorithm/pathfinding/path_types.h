#pragma once

#include "../types/grid_types.h"
#include "../types/geo_types.h"

// ================================================================
// A* Node Structure
// ================================================================
struct PathNode {
    GridCoordinate pos;          // Current position
    double g_cost;               // Cost from start
    double h_cost;               // Heuristic to goal
    double f_cost;               // g_cost + h_cost
    GridCoordinate parent_pos;   // Parent position
    double accumulated_time_hours; // Time accumulated (for fuel calculation)
    
    PathNode()
        : pos(-1, -1)
        , g_cost(0.0)
        , h_cost(0.0)
        , f_cost(0.0)
        , parent_pos(-1, -1)
        , accumulated_time_hours(0.0)
    {}
    
    PathNode(GridCoordinate p, double g, double h, GridCoordinate parent, double time = 0.0)
        : pos(p)
        , g_cost(g)
        , h_cost(h)
        , f_cost(g + h)
        , parent_pos(parent)
        , accumulated_time_hours(time)
    {}
};

// ================================================================
// Node Comparator for Priority Queue (min-heap)
// ================================================================
struct ComparePathNode {
    bool operator()(const PathNode& n1, const PathNode& n2) const {
        return n1.f_cost > n2.f_cost;  // Min-heap: smaller f_cost has higher priority
    }
};

// ================================================================
// Edge Cost Result
// ================================================================
struct EdgeCostResult {
    double cost;              // The value of cost for that edge
    double deltaTimeHours;    // The time taken to traverse that edge in hours
    
    EdgeCostResult() : cost(0.0), deltaTimeHours(0.0) {}
    EdgeCostResult(double c, double t) : cost(c), deltaTimeHours(t) {}
};

// ================================================================
// A* Search Result
// ================================================================
struct PathSearchResult {
    std::vector<GridCoordinate> path;  // Grid path
    double total_cost;                 // Total cost (distance or fuel)
    double total_time_hours;           // Total time in hours
    
    PathSearchResult()
        : total_cost(-1.0)
        , total_time_hours(0.0)
    {}
    
    bool IsSuccess() const {
        return total_cost >= 0.0 && !path.empty();
    }
};