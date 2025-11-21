#pragma once
#ifndef SHORT_PATH_H
#define SHORT_PATH_H

#include <vector>
#include <queue>
#include <map>
#include <limits>
#include <iostream>
#include "commapi.h"
#include "navigable_grid.h"
#include "common_path_utils.h"

SegmentResult aStarSearch(const NavigableGrid& navigable_grid, Point start, Point goal);
std::pair<FullRouteResults, double> findRouteWithWaypoints(
	const NavigableGrid& navigable_grid,
    const std::vector<Point>& waypoints);

#endif 