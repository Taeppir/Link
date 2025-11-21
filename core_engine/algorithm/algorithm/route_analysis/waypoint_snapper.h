#pragma once
#include "../types/grid_types.h"
#include "../types/geo_types.h"
#include "../results/route_results.h" 
#include <vector>
#include <string>

// ===== SnapResult 삭제, SnappingInfo 사용 =====

class IPortSnapper {
public:
    virtual ~IPortSnapper() = default;
    virtual bool SnapToPort(
        const GeoCoordinate& inputGeo,
        const NavigableGrid& grid,
        SnappingInfo& result 
    ) = 0;
};

class WaypointSnapper {
public:
    WaypointSnapper(
        const NavigableGrid& grid,
        IPortSnapper* portSnapper = nullptr
    );

    SnappingInfo SnapToNavigable( 
        const GeoCoordinate& inputGeo,
        double maxSearchRadiusKm = 50.0
    ) const;

    std::vector<SnappingInfo> SnapMultipleWaypoints(
        const std::vector<GeoCoordinate>& waypoints,
        double maxSearchRadiusKm = 50.0
    ) const;

    std::vector<GeoCoordinate> GetSnappedWaypoints(
        const std::vector<SnappingInfo>& results 
    ) const;

private:
    const NavigableGrid& navGrid_;
    IPortSnapper* portSnapper_;

    double CalculateDistanceKm(
        const GeoCoordinate& p1,
        const GeoCoordinate& p2
    ) const;

    GridCoordinate FindNearestNavigableCell(
        const GridCoordinate& start,
        double maxSearchRadiusKm
    ) const;
};