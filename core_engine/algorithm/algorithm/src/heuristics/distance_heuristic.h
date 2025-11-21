
#include "path_heuristic.h"
#include "../../include/great_circle_route.h"

class DistanceHeuristic : public IHeuristic {
public:
    double Estimate(
        const Point& from,
        const Point& goal,
        const NavigableGrid& grid
    ) const override;
};
