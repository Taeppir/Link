#include "../../include/common_path_utils.h"

Node::Node(Point p, double g, double h, Point parent, double time)
    : pos(p), g_cost(g), h_cost(h), f_cost(g + h), parent_pos(parent), accumulated_time_hours(time) {
}

bool CompareNode::operator()(const Node& n1, const Node& n2) const {
    return n1.f_cost > n2.f_cost;
}
