#pragma once
#include "../../include/navigable_grid.h"
#include "../../include/common_path_utils.h"

struct EdgeCostResult {
	double cost;	        // the value of cost for that edge
	double deltaTimeHours;  // the time taken to traverse that edge in hours
};

class ICostModel {
public: 
	virtual ~ICostModel() = default;

	virtual EdgeCostResult Compute(
		const Point& from,
		const Point& to,
		double accumulatedTimeHours
	) const = 0;
};