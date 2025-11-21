#pragma once
#include "navigable_grid.h"
#include "common_types.h"
#include <vector>
#include <string>

struct SnapResult {
    bool success;
    GeoCoordinate original;
    GeoCoordinate snapped;
    double distanceKm;
    std::string failureReason;

    SnapResult() : success(false), distanceKm(0.0) {}
};

class IPortSnapper {
public:
	virtual ~IPortSnapper() = default;
    virtual bool SnapToPort(
        const GeoCoordinate& inputGeo,
        const NavigableGrid& grid,
        SnapResult& result
    ) = 0;

};


class WaypointSnapper {
public:
    WaypointSnapper(
        const NavigableGrid& grid,
        IPortSnapper* portSnapper = nullptr
    );

    SnapResult SnapToNavigable(
        const GeoCoordinate& inputGeo,
        double maxSearchRadiusKm = 50.0
    ) const;

    std::vector<SnapResult> SnapMultipleWaypoints(
        const std::vector<GeoCoordinate>& waypoints,
        double maxSearchRadiusKm = 50.0
    ) const;

    std::vector<GeoCoordinate> GetSnappedWaypoints(
        const std::vector<SnapResult>& results
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