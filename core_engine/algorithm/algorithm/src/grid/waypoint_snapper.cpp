#include "../../include/waypoint_snapper.h"
#include "../../include/great_circle_route.h"
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
    // 거리 기준으로 가장 가까운 셀부터 탐색하기 위한 노드 구조
    struct Node {
        GridCoordinate pos;
        double distKmFromStart;
    };

    auto cmp = [](const Node& a, const Node& b) {
        return a.distKmFromStart > b.distKmFromStart; // min-heap
        };

    std::priority_queue<Node, std::vector<Node>, decltype(cmp)> pq(cmp);
    std::set<std::pair<int, int>> visited;

    GeoCoordinate startGeo = navGrid_.GridToGeo(start);

    // 시작 셀 추가
    pq.push(Node{ start, 0.0 });
    visited.insert({ start.row, start.col });

    // 8방향
    const int dr[] = { -1, -1, -1,  0, 0,  1, 1, 1 };
    const int dc[] = { -1,  0,  1, -1, 1, -1, 0, 1 };

    while (!pq.empty()) {
        Node current = pq.top();
        pq.pop();

        const GridCoordinate& curPos = current.pos;
        double curDist = current.distKmFromStart;

        // 최대 탐색 반경 초과하면 더 이상 진행 X
        if (curDist > maxSearchRadiusKm) {
            continue;
        }

        // 이 셀이 항해 가능하면 바로 반환
        if (navGrid_.IsNavigable(curPos.row, curPos.col)) {
            return curPos;
        }

        // 주변 이웃 탐색
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

    // Not found
    return GridCoordinate(-1, -1);
}


SnapResult WaypointSnapper::SnapToNavigable(
    const GeoCoordinate& inputGeo,
    double maxSearchRadiusKm) const
{
    SnapResult result;
    result.original = inputGeo;


    // ---------------------------------------------------------------
    // Step 1: 항만 기반 스냅 우선 시도
    // ---------------------------------------------------------------
    if (portSnapper_) {
		SnapResult portResult;
        if (portSnapper_->SnapToPort(inputGeo, navGrid_, portResult)) {
            return portResult;
        }
    }

    // ---------------------------------------------------------------
    // Step 2: Grid 기반 스냅
    // ---------------------------------------------------------------

    // Convert to grid
    GridCoordinate startGrid = navGrid_.GeoToGrid(inputGeo);

    // Already navigable?
    if (navGrid_.IsNavigable(startGrid.row, startGrid.col)) {
        result.success = true;
        result.snapped = inputGeo;
        result.distanceKm = 0.0;
        return result;
    }

    // Find nearest navigable cell
    GridCoordinate nearestGrid = FindNearestNavigableCell(
        startGrid,
        maxSearchRadiusKm
    );

    if (nearestGrid.row == -1) {
        result.success = false;
        result.failureReason = "No navigable cell found within " +
            std::to_string(maxSearchRadiusKm) + " km";
        return result;
    }

    // Success
    result.success = true;
    result.snapped = navGrid_.GridToGeo(nearestGrid);
    result.distanceKm = CalculateDistanceKm(inputGeo, result.snapped);

    return result;
}

std::vector<SnapResult> WaypointSnapper::SnapMultipleWaypoints(
    const std::vector<GeoCoordinate>& waypoints,
    double maxSearchRadiusKm) const
{
    std::vector<SnapResult> results;
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

        if (sr.success) {
            if (sr.distanceKm < 0.1) {
                std::cout << "Already navigable\n";
            }
            else {
                std::cout << "-> Snapped " << sr.distanceKm << " km to ("
                    << sr.snapped.latitude << ", "
                    << sr.snapped.longitude << ")\n";
            }
        }
        else {
            std::cerr << "FAILED: " << sr.failureReason << "\n";
        }
    }
#endif

    return results;
}

std::vector<GeoCoordinate> WaypointSnapper::GetSnappedWaypoints(
    const std::vector<SnapResult>& results) const
{
    std::vector<GeoCoordinate> snapped;
    snapped.reserve(results.size());

    for (const auto& r : results) {
        if (r.success) {
            snapped.push_back(r.snapped);
        }
    }

#ifdef _DEBUG
    std::cout << "Snap completeed for " << snapped.size()
		<< " waypoints." << std::endl;
#endif

    return snapped;
}