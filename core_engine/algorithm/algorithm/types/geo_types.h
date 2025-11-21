#pragma once
#include <vector>
#include <cmath>
#include <algorithm>

// ===== 기존 common_types.h에서 추출 =====

struct GeoCoordinate {
    double latitude;
    double longitude;

    GeoCoordinate() : latitude(0.0), longitude(0.0) {}
    GeoCoordinate(double lat, double lon) : latitude(lat), longitude(lon) {}
};

struct BoundingBox {
    double minLat, maxLat;
    double minLon, maxLon;

    BoundingBox() : minLat(0), maxLat(0), minLon(0), maxLon(0) {}

    BoundingBox(double minLat, double maxLat, double minLon, double maxLon)
        : minLat(minLat), maxLat(maxLat), minLon(minLon), maxLon(maxLon) {
    }

    bool Contains(const GeoCoordinate& point) const {
        return point.latitude >= minLat && point.latitude <= maxLat &&
            point.longitude >= minLon && point.longitude <= maxLon;
    }

    double Width() const { return maxLon - minLon; }
    double Height() const { return maxLat - minLat; }

    static BoundingBox FromWaypoints(const std::vector<GeoCoordinate>& waypoints);
};

struct GridCoordinate {
    int row;
    int col;

    GridCoordinate() : row(0), col(0) {}
    GridCoordinate(int r, int c) : row(r), col(c) {}

    bool operator==(const GridCoordinate& other) const {
        return row == other.row && col == other.col;
    }

    bool operator<(const GridCoordinate& other) const {
        if (row != other.row) {
            return row < other.row;
        }
        return col < other.col;
    }
};