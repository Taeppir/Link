#include "legacy_fuel_cost_model.h"

LegacyFuelCostModel::LegacyFuelCostModel(
    const NavigableGrid& grid,
    VoyageInfo voyageInfoBase,
    unsigned int startTimeSec,
    const std::map<std::string, WeatherDataInput>& weatherData,
    double shipSpeedMps)
    : grid_(grid),
    voyageInfoBase_(voyageInfoBase),
    startTimeSec_(startTimeSec),
    weatherData_(weatherData),
    shipSpeedMps_(shipSpeedMps)
{
}

EdgeCostResult LegacyFuelCostModel::Compute(
    const Point& from,
    const Point& to,
    double accumulatedTimeHours
) const
{
	// 1) transform grid -> geo
    GeoCoordinate fromGeo = grid_.GridToGeo(from.row, from.col);
    GeoCoordinate toGeo = grid_.GridToGeo(to.row, to.col);

	// 2) distance [km] and time [hours] between from/to
    double distKm = greatCircleDistance(
        fromGeo.latitude, fromGeo.longitude,
        toGeo.latitude, toGeo.longitude
    );

    double timeHours = timeCalculator(distKm, shipSpeedMps_);

    // 3) mid point
    GeoCoordinate midGeo = {
        (fromGeo.latitude + toGeo.latitude) / 2.0,
        (fromGeo.longitude + toGeo.longitude) / 2.0
    };

	// 4) calculate heading
    VoyageInfo vInfo = voyageInfoBase_;
    vInfo.heading = calculateBearing(fromGeo, toGeo);

	// 5) mid time in seconds
    unsigned int midTimeSec = startTimeSec_
        + static_cast<unsigned int>(
            (accumulatedTimeHours + timeHours / 2.0) * 3600.0
            );

	// 6) calculate fuel rate [kg/h] at mid point & mid time
    double fuelRateKgPerHour = fuelCalculator(
        midTimeSec,
        midGeo.latitude, midGeo.longitude,
        weatherData_,
        vInfo
    );

	// 7) fuel consumption [kg] for this edge
    double fuelKg = fuelRateKgPerHour * timeHours;

    EdgeCostResult r;
    r.cost = fuelKg;
    r.deltaTimeHours = timeHours;
    return r;
}
