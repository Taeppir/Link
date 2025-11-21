#include <cmath>
#include "../../include/common_path_utils.h" 

bool angleCheck(const Node& current_node, int dx_curr, int dy_curr) {

    Point current_pos = current_node.pos;
    Point parent_pos = current_node.parent_pos;

    if (parent_pos.row == -1) {
        return true;
    }

    int dx_prev = current_pos.row - parent_pos.row;
    int dy_prev = current_pos.col - parent_pos.col;

    double mag_prev = std::sqrt((double)dx_prev * dx_prev + (double)dy_prev * dy_prev);
    double mag_curr = std::sqrt((double)dx_curr * dx_curr + (double)dy_curr * dy_curr);
    if (mag_prev == 0.0) {
        return true;
    }
    if (mag_curr == 0.0) {
        return false;
    }

    double dot_product = (double)dx_prev * dx_curr + (double)dy_prev * dy_curr;

    double cos_theta = dot_product / (mag_prev * mag_curr);

    if (cos_theta > 1.0) cos_theta = 1.0;
    if (cos_theta < -1.0) cos_theta = -1.0;

    double angle_rad = std::acos(cos_theta);
    double angle_change = angle_rad * 180.0 / PI;

    return angle_change <= MAX_ANGLE_DEGREES;
}
