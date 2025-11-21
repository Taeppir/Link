#pragma once

#include "path_types.h"
#include "../types/grid_types.h"

// ================================================================
// Constants
// ================================================================
constexpr double MAX_ANGLE_DEGREES = 90.0;
constexpr double PI = 3.14159265358979323846;

// ================================================================
// 8-Direction Movement
// ================================================================
constexpr int DX_8DIR[8] = { -1, -1, -1,  0,  0,  1,  1,  1 };
constexpr int DY_8DIR[8] = { -1,  0,  1, -1,  1, -1,  0,  1 };

// ================================================================
// Grid Validation
// ================================================================

/**
 * @brief Check if a grid position is valid and navigable
 */
bool IsValidAndNavigable(
    const NavigableGrid& grid,
    int row,
    int col
);

bool IsValidAndNavigable(
    const NavigableGrid& grid,
    const GridCoordinate& pos
);

// ================================================================
// Angle Limiting
// ================================================================

/**
 * @brief Check if the angle change is within limits
 * 
 * Prevents sharp turns by checking if the angle between
 * the previous movement direction and current movement direction
 * is within MAX_ANGLE_DEGREES (90 degrees).
 * 
 * @param current_node Current node with parent information
 * @param dx_curr Current movement direction (row delta)
 * @param dy_curr Current movement direction (col delta)
 * @return true if angle is within limits, false otherwise
 */
bool AngleCheck(
    const PathNode& current_node,
    int dx_curr,
    int dy_curr
);