#include "path_utils.h"
#include <cmath>

bool IsValidAndNavigable(
    const NavigableGrid& grid,
    int row,
    int col)
{
    return grid.IsValid(row, col) && grid.IsNavigable(row, col);
}

bool IsValidAndNavigable(
    const NavigableGrid& grid,
    const GridCoordinate& pos)
{
    return IsValidAndNavigable(grid, pos.row, pos.col);
}

bool AngleCheck(
    const PathNode& current_node,
    int dx_curr,
    int dy_curr)
{
    GridCoordinate current_pos = current_node.pos;
    GridCoordinate parent_pos = current_node.parent_pos;
    
    // First move: no parent, always valid
    if (parent_pos.row == -1) {
        return true;
    }
    
    // Calculate previous movement direction
    int dx_prev = current_pos.row - parent_pos.row;
    int dy_prev = current_pos.col - parent_pos.col;
    
    // Calculate magnitudes
    double mag_prev = std::sqrt((double)dx_prev * dx_prev + (double)dy_prev * dy_prev);
    double mag_curr = std::sqrt((double)dx_curr * dx_curr + (double)dy_curr * dy_curr);
    
    // Edge cases
    if (mag_prev == 0.0) {
        return true;  // No previous movement
    }
    if (mag_curr == 0.0) {
        return false; // No current movement (invalid)
    }
    
    // Calculate dot product
    double dot_product = (double)dx_prev * dx_curr + (double)dy_prev * dy_curr;
    
    // Calculate cosine of angle
    double cos_theta = dot_product / (mag_prev * mag_curr);
    
    // Clamp to valid range [-1, 1]
    if (cos_theta > 1.0) cos_theta = 1.0;
    if (cos_theta < -1.0) cos_theta = -1.0;
    
    // Calculate angle in degrees
    double angle_rad = std::acos(cos_theta);
    double angle_change = angle_rad * 180.0 / PI;
    
    // Check if within limit
    return angle_change <= MAX_ANGLE_DEGREES;
}