#pragma once

#include "path_heuristic.h"
#include "../../include/common_types.h"
#include "../../include/navigable_grid.h"

class FuelHeuristic : public IHeuristic {
public:
    FuelHeuristic(double minFuelRateKgPerHour,
        double shipSpeedMps,
        const GeoCoordinate& goalGeo);

    double Estimate(
        const Point& from,
        const Point& goal,
        const NavigableGrid& grid
    ) const override;

private:
    double        minFuelRateKgPerHour_;
    double        shipSpeedMps_;
    GeoCoordinate goalGeo_;
};
