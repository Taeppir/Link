#include "fuel_heuristic.h"

#include "../../include/great_circle_route.h"
#include "../../include/less_fuel.h"

FuelHeuristic::FuelHeuristic(
    double minFuelRateKgPerHour,
    double shipSpeedMps,
    const GeoCoordinate& goalGeo)
    : minFuelRateKgPerHour_(minFuelRateKgPerHour),
    shipSpeedMps_(shipSpeedMps),
    goalGeo_(goalGeo)
{
}

double FuelHeuristic::Estimate(
    const Point& from,
    const Point& /*goal*/,
    const NavigableGrid& grid
) const
{
    GeoCoordinate fromGeo = grid.GridToGeo(from.row, from.col);

    double distKm = greatCircleDistance(
        fromGeo.latitude, fromGeo.longitude,
        goalGeo_.latitude, goalGeo_.longitude
    );

    double timeHours = timeCalculator(distKm, shipSpeedMps_);

    return minFuelRateKgPerHour_ * timeHours;
}
