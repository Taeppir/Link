#pragma once
#ifndef COMMON_PATH_UTILS_H
#define COMMON_PATH_UTILS_H

#include <vector>
#include <queue> 
#include "navigable_grid.h" 
#include "great_circle_route.h" 

static bool isValid(const NavigableGrid& grid, int x, int y) {
    // grid 경계 내에 있는지, 'NAVIGABLE' 셀인지 확인
    return grid.IsValid(x, y) && grid.IsNavigable(x, y);
}

using Point = GridCoordinate;

struct Node {
    Point pos;      
    double g_cost;  
    double h_cost;  
    double f_cost;  
    Point parent_pos;  
    double accumulated_time_hours;

    Node(Point p, double g, double h, Point parent, double time = 0.0);
};

using SegmentResult = std::pair<std::vector<Point>, double>;
using FullRouteResults = std::vector<SegmentResult>;

struct CompareNode {
    bool operator()(const Node& n1, const Node& n2) const;
};

inline constexpr double MAX_ANGLE_DEGREES = 60.0;

//// 8방향 탐색
//const int DX[] = { 0, 0, 1, -1, 1, -1, 1, -1 };
//const int DY[] = { 1, -1, 0, 0, 1, -1, -1, 1 };
//const int directions = 8;

// 16방향 탐색
const int DX[] = { 0, 0, 1, -1, 1, -1, 1, -1, 2, 2, 1, 1, -2, -2, -1, -1 };
const int DY[] = { 1, -1, 0, 0, 1, -1, -1, 1, 1, -1, 2, -2, 1, -1, 2, -2 };
const int directions = 16;

bool angleCheck(const Node& current_node, int dx_curr, int dy_curr);

#endif 