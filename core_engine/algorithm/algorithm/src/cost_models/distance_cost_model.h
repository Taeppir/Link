#pragma once
#include "path_cost_model.h"
#include "../../include/navigable_grid.h"
#include "../../include/great_circle_route.h"
#include "../../include/time_calculator.h"

class DistanceCostModel : public ICostModel {
public:
    DistanceCostModel(const NavigableGrid& grid, double shipSpeedMps)
        : grid_(grid), shipSpeedMps_(shipSpeedMps) {
    }

    EdgeCostResult Compute(
        const Point& from,
        const Point& to,
        double /*accumulatedTimeHours*/) const override;

private:
    const NavigableGrid& grid_;
    double shipSpeedMps_;
};
