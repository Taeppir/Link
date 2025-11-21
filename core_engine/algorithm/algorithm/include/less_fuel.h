#pragma once
#ifndef LESS_FUEL_H
#define LESS_FUEL_H

#include <vector>
#include <queue>
#include <map>
#include <limits>
#include <iostream>
#include "commapi.h"
#include "navigable_grid.h"
#include "common_path_utils.h"
#include "fuel_consumption_calculator.h"
#include "read_weather_data.h"
#include "time_calculator.h"
#include "geo_utils.h"

struct OptimizedSegmentResult {
	std::vector<Point> path;
	double fuel_cost;
	double total_time_hours; 
};

OptimizedSegmentResult aStarSearch_optimized(
	const NavigableGrid& navigable_grid, 
	Point start, 
	Point goal, 
	VoyageInfo voyageInfo, 
	unsigned int start_time, 
	const std::map<std::string, WeatherDataInput>& weather_data
);

std::pair<FullRouteResults, double> findRouteWithWaypoints_optimized(
	const NavigableGrid& navigable_grid,
	const std::vector<Point>& waypoints,
	VoyageInfo voyageInfo,
	unsigned int start_time,
	const std::map<std::string, WeatherDataInput>& weather_data
);

#endif 