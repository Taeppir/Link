#include "waypoint_snapper.h"
#include "../utils/geo_calculations.h"
#include <queue>
#include <set>
#include <cmath>
#include <iostream>

WaypointSnapper::WaypointSnapper(
    const NavigableGrid& grid,
    IPortSnapper* portSnapper
    )
    : navGrid_(grid), portSnapper_(portSnapper)
{
}

double WaypointSnapper::CalculateDistanceKm(
    const GeoCoordinate& p1,
    const GeoCoordinate& p2) const
{
    return greatCircleDistance(
        p1.latitude, p1.longitude,
        p2.latitude, p2.longitude
    );
}

GridCoordinate WaypointSnapper::FindNearestNavigableCell(
    const GridCoordinate& start,
    double maxSearchRadiusKm) const
{
    struct Node {
        GridCoordinate pos;
        double distKmFromStart;
    };

    auto cmp = [](const Node& a, const Node& b) {
        return a.distKmFromStart > b.distKmFromStart;
    };

    std::priority_queue<Node, std::vector<Node>, decltype(cmp)> pq(cmp);
    std::set<std::pair<int, int>> visited;

    GeoCoordinate startGeo = navGrid_.GridToGeo(start);

    pq.push(Node{ start, 0.0 });
    visited.insert({ start.row, start.col });

    const int dr[] = { -1, -1, -1,  0, 0,  1, 1, 1 };
    const int dc[] = { -1,  0,  1, -1, 1, -1, 0, 1 };

    while (!pq.empty()) {
        Node current = pq.top();
        pq.pop();

        const GridCoordinate& curPos = current.pos;
        double curDist = current.distKmFromStart;

        if (curDist > maxSearchRadiusKm) {
            continue;
        }

        if (navGrid_.IsNavigable(curPos.row, curPos.col)) {
            return curPos;
        }

        for (int i = 0; i < 8; ++i) {
            int nr = curPos.row + dr[i];
            int nc = curPos.col + dc[i];

            if (!navGrid_.IsValid(nr, nc)) continue;
            if (visited.count({ nr, nc })) continue;

            GridCoordinate neighbor(nr, nc);
            GeoCoordinate neighborGeo = navGrid_.GridToGeo(neighbor);
            double distKm = CalculateDistanceKm(startGeo, neighborGeo);

            if (distKm > maxSearchRadiusKm) {
                continue;
            }

            visited.insert({ nr, nc });
            pq.push(Node{ neighbor, distKm });
        }
    }

    return GridCoordinate(-1, -1);
}

SnappingInfo WaypointSnapper::SnapToNavigable(
    const GeoCoordinate& inputGeo,
    double maxSearchRadiusKm) const
{
    SnappingInfo result;
    result.original = inputGeo;

    // Step 1: 항만 기반 스냅 우선 시도
    if (portSnapper_) {
        SnappingInfo portResult; 
        if (portSnapper_->SnapToPort(inputGeo, navGrid_, portResult)) {
            return portResult;
        }
    }

    // Step 2: Grid 기반 스냅
    GridCoordinate startGrid = navGrid_.GeoToGrid(inputGeo);

    if (navGrid_.IsNavigable(startGrid.row, startGrid.col)) {
        result.status = SnappingStatus::ALREADY_NAVIGABLE;
        result.was_snapped = false; 
        result.snapped = inputGeo;
        result.snapping_distance_km = 0.0;
        return result;
    }

    GridCoordinate nearestGrid = FindNearestNavigableCell(
        startGrid,
        maxSearchRadiusKm
    );

    if (nearestGrid.row == -1) {
        result.status = SnappingStatus::FAILED;
        result.was_snapped = false;
        result.failure_reason = "No navigable cell found within " +
            std::to_string(maxSearchRadiusKm) + " km";  
        return result;
    }

    result.status = SnappingStatus::SNAPPED;
    result.was_snapped = true;
    result.snapped = navGrid_.GridToGeo(nearestGrid);
    result.snapping_distance_km = CalculateDistanceKm(inputGeo, result.snapped); 

    return result;
}

std::vector<SnappingInfo> WaypointSnapper::SnapMultipleWaypoints( 
    const std::vector<GeoCoordinate>& waypoints,
    double maxSearchRadiusKm) const
{
    std::vector<SnappingInfo> results;  
    results.reserve(waypoints.size());

    for (const auto& wp : waypoints) {
        results.push_back(SnapToNavigable(wp, maxSearchRadiusKm));
    }

#ifdef _DEBUG
    for (size_t i = 0; i < results.size(); ++i) {
        const auto& sr = results[i];
        std::cout << "Waypoint " << i << ": ("
            << sr.original.latitude << ", "
            << sr.original.longitude << ") ";

        switch (sr.status) {
        case SnappingStatus::ALREADY_NAVIGABLE:
            std::cout << "Already navigable\n";
            break;
        case SnappingStatus::SNAPPED:
            std::cout << "-> Snapped " << sr.snapping_distance_km << " km to ("
                << sr.snapped.latitude << ", "
                << sr.snapped.longitude << ")\n";
            break;
        case SnappingStatus::FAILED:
            std::cerr << "FAILED: " << sr.failure_reason << "\n";
            break;
        }
    }
#endif

    return results;
}

std::vector<GeoCoordinate> WaypointSnapper::GetSnappedWaypoints(
    const std::vector<SnappingInfo>& results) const 
{
    std::vector<GeoCoordinate> snapped;
    snapped.reserve(results.size());

    for (const auto& r : results) {
        if (r.IsSuccess()) { 
            snapped.push_back(r.snapped);
        }
    }

#ifdef _DEBUG
    std::cout << "Snap completed for " << snapped.size()
        << " waypoints." << std::endl;
#endif

    return snapped;
}