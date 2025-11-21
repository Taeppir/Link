#include "../../include/navigable_grid.h"
#include "../../include/short_path.h"

std::pair<FullRouteResults, double> findRouteWithWaypoints(
	const NavigableGrid& navigable_grid,
    const std::vector<Point>& waypoints)
{
    if (waypoints.size() < 2) {
        return { {}, 0.0 };
    }

    FullRouteResults all_segment_results;
    double total_distance = 0.0;

    for (size_t i = 0; i < waypoints.size() - 1; ++i) {
        Point start = waypoints[i];
        Point goal = waypoints[i + 1];

        std::pair<std::vector<Point>, double> result = aStarSearch(navigable_grid ,start, goal);
        std::vector<Point> segment_path = result.first;
        double segment_cost = result.second;

        if (segment_cost < 0) {
            std::cerr << "Error: Path not found from (" << start.row << ", " << start.col
                << ") to (" << goal.row << ", " << goal.col << ")." << std::endl;
            return { {}, -1.0 };
        }

        all_segment_results.push_back({ segment_path, segment_cost });
        total_distance += segment_cost;
    }

    return { all_segment_results, total_distance };
}