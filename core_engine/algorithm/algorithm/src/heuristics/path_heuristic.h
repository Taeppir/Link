#pragma once
#include "../../include/navigable_grid.h"
#include "../../include/common_path_utils.h"

class IHeuristic {
public:
    virtual ~IHeuristic() = default;

    virtual double Estimate(
        const Point& from,
        const Point& goal,
        const NavigableGrid& grid
    ) const = 0;
};
