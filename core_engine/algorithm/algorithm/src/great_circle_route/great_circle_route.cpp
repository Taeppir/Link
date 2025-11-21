#include "../../include/great_circle_route.h"
#include <iostream>
#include <cmath>

double greatCircleDistance(double lat1, double lon1, double lat2, double lon2) {

    double R = 6371.0;

    lat1 = lat1 * (PI / 180.0);
    lon1 = lon1 * (PI / 180.0);
    lat2 = lat2 * (PI / 180.0);
    lon2 = lon2 * (PI / 180.0);
    double dLat = lat2 - lat1;
    double dLon = lon2 - lon1;

    double a = std::pow(std::sin(dLat / 2), 2) +
        std::cos(lat1) * std::cos(lat2) * std::pow(std::sin(dLon / 2), 2);
    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));

    double distance = R * c;

    return distance;
}

// 대권 항로를 따라 중간 지점들을 생성하는 함수
std::vector<GeoCoordinate> generateGreatCirclePoints(
    GeoCoordinate start, GeoCoordinate end, int num_points)
{
    std::vector<GeoCoordinate> points;

    double lat1 = start.latitude * PI / 180.0;
    double lon1 = start.longitude * PI / 180.0;
    double lat2 = end.latitude * PI / 180.0;
    double lon2 = end.longitude * PI / 180.0;

    double d = greatCircleDistance(start.latitude, start.longitude, end.latitude, end.longitude) / 6371.0; // 각거리

    for (int i = 0; i <= num_points; ++i) {
        double f = static_cast<double>(i) / num_points;
        double A = std::sin((1 - f) * d) / std::sin(d);
        double B = std::sin(f * d) / std::sin(d);

        double x = A * std::cos(lat1) * std::cos(lon1) + B * std::cos(lat2) * std::cos(lon2);
        double y = A * std::cos(lat1) * std::sin(lon1) + B * std::cos(lat2) * std::sin(lon2);
        double z = A * std::sin(lat1) + B * std::sin(lat2);

        double lat_i = std::atan2(z, std::sqrt(x * x + y * y));
        double lon_i = std::atan2(y, x);

        points.emplace_back(lat_i * 180.0 / PI, lon_i * 180.0 / PI);
    }
    return points;
}