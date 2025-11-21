#include "distance_heuristic.h"

double DistanceHeuristic::Estimate(
    const Point& from,
    const Point& goal,
    const NavigableGrid& grid
) const
{
    GeoCoordinate a = grid.GridToGeo(from.row, from.col);
    GeoCoordinate b = grid.GridToGeo(goal.row, goal.col);

    double distKm = greatCircleDistance(
        a.latitude, a.longitude,
        b.latitude, b.longitude
    );
    return distKm;
}
