#include <cmath>
#include <algorithm> 

#include "../../include/less_fuel.h"
#include "../cost_models/legacy_fuel_cost_model.h"
#include "../heuristics/fuel_heuristic.h"


OptimizedSegmentResult aStarSearch_optimized(const NavigableGrid& navigable_grid, Point start, Point goal,
    VoyageInfo voyageInfo,
    unsigned int start_time,
    const std::map<std::string, WeatherDataInput>& weather_data)
{
    const double SHIP_SPEED_MPS = 8.0;

    // 1. check start/goal validity
    if (!isValid(navigable_grid, start.row, start.col)
        || !isValid(navigable_grid, goal.row, goal.col)) {
        std::cerr << "Error: Start or Goal position is not navigable." << std::endl;
        return { {}, -1.0, 0.0 };
    }

    if (start == goal) {
        return { {start}, 0.0, 0.0 };
    }

    // 2. A* basic structures
    std::map<Point, double> g_scores;
    g_scores[start] = 0.0;

    std::map<Point, Point> parents;
    std::map<Point, bool> closed_list;
    std::priority_queue<Node, std::vector<Node>, CompareNode> open_list;

	// 3. transform start/goal to geo
    GeoCoordinate start_geo = navigable_grid.GridToGeo(start.row, start.col);
    GeoCoordinate goal_geo = navigable_grid.GridToGeo(goal.row, goal.col);

	// 4. initial heuristic calculation
    double dist_km = greatCircleDistance(start_geo.latitude, start_geo.longitude, goal_geo.latitude, goal_geo.longitude);
    double time_hours = timeCalculator(dist_km, SHIP_SPEED_MPS);
    voyageInfo.heading = calculateBearing(start_geo, goal_geo);

	// ideal fuel rate at start point
	// TODO: why at start point? not mid point?
    double min_fuel_rate_kg_per_hour = fuelCalculator_zero(
        start_time,
        start_geo.latitude, start_geo.longitude,
        voyageInfo);

	// 5. construct costModel and Huristic
    LegacyFuelCostModel costModel(
        navigable_grid,
        voyageInfo,
        start_time,
        weather_data,
        SHIP_SPEED_MPS
    );

    FuelHeuristic heuristic(
        min_fuel_rate_kg_per_hour,
        SHIP_SPEED_MPS,
        goal_geo
    );


	double initial_h = heuristic.Estimate(start, goal, navigable_grid);
    Node start_node(start, 0.0, initial_h, { -1, -1 }, 0.0); // accumulated_time_hours = 0
    open_list.push(start_node);


	// 6. A* main loop
    while (!open_list.empty()) {
        Node current = open_list.top();
        open_list.pop();

        Point current_pos = current.pos;

        if (closed_list.count(current_pos)) {
            continue;
        }

        if (current_pos == goal) {
            std::vector<Point> path;
            Point p = goal;
            double total_cost = current.g_cost;
            double final_time_hours = current.accumulated_time_hours;

            while (parents.count(p)) {
                path.push_back(p);
                p = parents[p];
            }
            path.push_back(start);
            std::reverse(path.begin(), path.end());

            return { path, total_cost, final_time_hours };
        }

        closed_list[current_pos] = true;

        double accumulated_time_hours = current.accumulated_time_hours;

        for (int i = 0; i < directions; ++i) {
            int new_x = current_pos.row + DX[i];
            int new_y = current_pos.col + DY[i];
            Point neighbor_pos = { new_x, new_y };


            if (!isValid(navigable_grid, new_x, new_y)) {
                continue;
			}

            if (closed_list.count(neighbor_pos)) {
                continue;
            }

            if (!angleCheck(current, DX[i], DY[i])) {
                continue;
			}

            EdgeCostResult edge = costModel.Compute(
                current_pos,
                neighbor_pos,
                accumulated_time_hours
            );

			double travel_cost = edge.cost;
			double time_hours_g = edge.deltaTimeHours;

			double new_g_cost = current.g_cost + travel_cost;

            double current_best_g_cost = g_scores.count(neighbor_pos) ?
                g_scores[neighbor_pos] :
				std::numeric_limits<double>::infinity();

            if (new_g_cost < current_best_g_cost) {
                g_scores[neighbor_pos] = new_g_cost;
                parents[neighbor_pos] = current_pos;

                double h_cost = heuristic.Estimate(
                    neighbor_pos,
                    goal,
                    navigable_grid
                );

                double neighbor_total_time_hours = current.accumulated_time_hours + time_hours_g;
                
                Node neighbor_node(
                    neighbor_pos,
                    new_g_cost,
                    h_cost,
                    current_pos,
                    neighbor_total_time_hours
                );

                open_list.push(neighbor_node);
			}
        }
    }

    return { {}, -1.0 , -1.0 };
}