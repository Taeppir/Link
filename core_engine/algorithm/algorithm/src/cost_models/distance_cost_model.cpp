#include "distance_cost_model.h"

EdgeCostResult DistanceCostModel::Compute(
    const Point& from,
    const Point& to,
    double /*accumulatedTimeHours*/) const
{
    GeoCoordinate a = grid_.GridToGeo(from.row, from.col);
    GeoCoordinate b = grid_.GridToGeo(to.row, to.col);

    double distKm = greatCircleDistance(
        a.latitude, a.longitude,
        b.latitude, b.longitude
    );

    double timeHours = timeCalculator(distKm, shipSpeedMps_);

    EdgeCostResult r;
    r.cost = distKm;
    r.deltaTimeHours = timeHours;
    return r;
}
