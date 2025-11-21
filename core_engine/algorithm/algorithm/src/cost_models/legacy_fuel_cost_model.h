#pragma once

#include "path_cost_model.h"
#include "../../include/common_types.h"
#include "../../include/navigable_grid.h"
#include "../../include/read_weather_data.h" 
#include "../../include/less_fuel.h"
#include "../../include/great_circle_route.h"


class LegacyFuelCostModel : public ICostModel {
public:
    LegacyFuelCostModel(
        const NavigableGrid& grid,
        VoyageInfo voyageInfoBase,
        unsigned int startTimeSec,
        const std::map<std::string, WeatherDataInput>& weatherData,
        double shipSpeedMps
    );

    EdgeCostResult Compute(
        const Point& from,
        const Point& to,
        double accumulatedTimeHours
    ) const override;

private:
    const NavigableGrid& grid_;
	VoyageInfo voyageInfoBase_;   // base voyage info except heading (later, we need to exclude ship speed also)
	unsigned int startTimeSec_;   // symulation start time in seconds
    const std::map<std::string, WeatherDataInput>& weatherData_;
    double shipSpeedMps_;
};
