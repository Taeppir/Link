#include "../../include/navigable_grid.h"
#include "../../include/less_fuel.h"

std::pair<FullRouteResults, double> findRouteWithWaypoints_optimized(
    const NavigableGrid& navigable_grid,
    const std::vector<Point>& waypoints,
    VoyageInfo voyageInfo,
    unsigned int start_time,
    const std::map<std::string, WeatherDataInput>& weather_data)
{
    if (waypoints.size() < 2) {
        return { {}, 0.0 };
    }

    FullRouteResults all_segment_results;
    double total_fuel_consumption = 0.0;
    unsigned int current_segment_start_time = start_time;

    for (size_t i = 0; i < waypoints.size() - 1; ++i) {
        Point start = waypoints[i];
        Point goal = waypoints[i + 1];

        OptimizedSegmentResult result = aStarSearch_optimized(navigable_grid, start, goal, voyageInfo, current_segment_start_time, weather_data);
        std::vector<Point> segment_path = result.path;
        double segment_cost = result.fuel_cost;

        if (segment_cost < 0) {
            std::cerr << "Error: Path not found from (" << start.row << ", " << start.col
                << ") to (" << goal.row << ", " << goal.col << ")." << std::endl;
            return { {}, -1.0 };
        }

        double segment_time_hours = result.total_time_hours;
        current_segment_start_time += static_cast<unsigned int>(segment_time_hours * 3600.0);

        all_segment_results.push_back({ segment_path, segment_cost });
        total_fuel_consumption += segment_cost;
    }

    return { all_segment_results, total_fuel_consumption / 1000.0 };
}