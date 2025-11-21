#include "geo_calculations.h"
#include <cmath>

// ===== 기존 great_circle_route.cpp 내용 복사 =====

double greatCircleDistance(double lat1, double lon1, double lat2, double lon2) {
    const double R = 6371.0; // Earth radius in km

    double lat1_rad = lat1 * PI / 180.0;
    double lat2_rad = lat2 * PI / 180.0;
    double delta_lat = (lat2 - lat1) * PI / 180.0;
    double delta_lon = (lon2 - lon1) * PI / 180.0;

    double a = std::sin(delta_lat / 2.0) * std::sin(delta_lat / 2.0) +
               std::cos(lat1_rad) * std::cos(lat2_rad) *
               std::sin(delta_lon / 2.0) * std::sin(delta_lon / 2.0);

    double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));

    return R * c;
}

std::vector<GeoCoordinate> generateGreatCirclePoints(
    GeoCoordinate start, 
    GeoCoordinate end, 
    int num_points)
{
    std::vector<GeoCoordinate> points;
    
    if (num_points < 2) {
        return points;
    }

    double lat1 = start.latitude * PI / 180.0;
    double lon1 = start.longitude * PI / 180.0;
    double lat2 = end.latitude * PI / 180.0;
    double lon2 = end.longitude * PI / 180.0;

    for (int i = 0; i < num_points; ++i) {
        double f = static_cast<double>(i) / (num_points - 1);
        
        double a = std::sin((1.0 - f) * 1.0) / std::sin(1.0);
        double b = std::sin(f * 1.0) / std::sin(1.0);
        
        double x = a * std::cos(lat1) * std::cos(lon1) + b * std::cos(lat2) * std::cos(lon2);
        double y = a * std::cos(lat1) * std::sin(lon1) + b * std::cos(lat2) * std::sin(lon2);
        double z = a * std::sin(lat1) + b * std::sin(lat2);
        
        double lat = std::atan2(z, std::sqrt(x * x + y * y));
        double lon = std::atan2(y, x);
        
        points.push_back(GeoCoordinate(lat * 180.0 / PI, lon * 180.0 / PI));
    }
    
    return points;
}

// ===== 기존 geo_utils.cpp 내용 복사 (calculateBearing) =====

double calculateBearing(const GeoCoordinate& from, const GeoCoordinate& to) {
    double lat1 = from.latitude * PI / 180.0;
    double lat2 = to.latitude * PI / 180.0;
    double dLon = (to.longitude - from.longitude) * PI / 180.0;

    double y = std::sin(dLon) * std::cos(lat2);
    double x = std::cos(lat1) * std::sin(lat2) -
               std::sin(lat1) * std::cos(lat2) * std::cos(dLon);

    double bearing_rad = std::atan2(y, x);
    double bearing_deg = bearing_rad * 180.0 / PI;

    // Normalize to 0-360
    bearing_deg = std::fmod(bearing_deg + 360.0, 360.0);

    return bearing_deg;
}