#include <cmath>
#include <algorithm> 

#include "../../include/short_path.h"
#include "../cost_models/distance_cost_model.h"
#include "../heuristics/distance_heuristic.h"

SegmentResult aStarSearch(const NavigableGrid& navigable_grid, Point start, Point goal) {

	// 1. check start/goal validity
    if (!isValid(navigable_grid, start.row, start.col)
        || !isValid(navigable_grid, goal.row, goal.col)) {
        std::cerr << "Error: Start or Goal position is not navigable." << std::endl;
        return { {}, -1.0 };
    }

    if (start == goal) {
        return { {start}, 0.0 };
    }

	// 2. A* basic structures
    std::map<Point, double> g_scores;
    g_scores[start] = 0.0;

    std::map<Point, Point> parents;
    std::map<Point, bool> closed_list;
    std::priority_queue<Node, std::vector<Node>, CompareNode> open_list;

	// 3. cost model & heuristic model
	const double SHIP_SPEED_MPS = 8.0;

	DistanceCostModel cost_model(navigable_grid, SHIP_SPEED_MPS);
    DistanceHeuristic heuristic;

	double initial_h = heuristic.Estimate(start, goal, navigable_grid);
    Node start_node(start, 0.0, initial_h, { -1, -1 });
    open_list.push(start_node);

	// 4. A* main loop
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

            while (parents.count(p)) {
                path.push_back(p);
                p = parents[p];
            }
            path.push_back(start);
            std::reverse(path.begin(), path.end());

            return { path, total_cost };
        }

        closed_list[current_pos] = true;

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

			EdgeCostResult edge_cost = cost_model.Compute(
                current_pos,
                neighbor_pos,
				/*accumulatedTimeHours*/ 0.0   // not used in DistanceCostModel
            );

			double travel_cost = edge_cost.cost;
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

                Node neighbor_node(
                    neighbor_pos,
                    new_g_cost,
                    h_cost,
                    current_pos
                );

                open_list.push(neighbor_node);
			}
        }
    }
    return { {}, -1.0 };
}