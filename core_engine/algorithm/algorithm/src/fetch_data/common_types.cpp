#include "../../include/common_types.h"
#include <cmath>


// calculate bounding box from waypoints(witthout margin)
BoundingBox BoundingBox::FromWaypoints(const std::vector<GeoCoordinate>& waypoints) {
    if (waypoints.empty()) {
        return BoundingBox();
    }

    double minLat = waypoints[0].latitude;
    double maxLat = waypoints[0].latitude;
    double minLon = waypoints[0].longitude;
    double maxLon = waypoints[0].longitude;

    for (const auto& point : waypoints) {
        minLat = std::min(minLat, point.latitude);
        maxLat = std::max(maxLat, point.latitude);
        minLon = std::min(minLon, point.longitude);
        maxLon = std::max(maxLon, point.longitude);
    }

    return BoundingBox(
        minLat,
        maxLat,
        minLon,
        maxLon
    );
}