#pragma once
#ifndef GREAT_CIRCLE_ROUTE_H
#define GREAT_CIRCLE_ROUTE_H

#include <vector>
#include "common_types.h"

inline constexpr double PI = 3.14159265358979323846;

double greatCircleDistance(double lat1, double lon1, double lat2, double lon2);

std::vector<GeoCoordinate> generateGreatCirclePoints(
    GeoCoordinate start, GeoCoordinate end, int num_points = 50);

#endif