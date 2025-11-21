#pragma once
#include <cmath>
#include "common_types.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * @brief 각도를 라디안으로 변환
 */
inline double toRadians(double degrees) {
    return degrees * M_PI / 180.0;
}

/**
 * @brief 라디안을 각도로 변환
 */
inline double toDegrees(double radians) {
    return radians * 180.0 / M_PI;
}

/**
 * @brief 두 지리 좌표 간의 초기 방위각(heading) 계산
 * @param start 시작 지점 (GeoCoordinate)
 * @param end 도착 지점 (GeoCoordinate)
 * @return 0-360도 범위의 방위각 (북쪽 0도, 시계 방향)
 */
inline double calculateBearing(const GeoCoordinate& start, const GeoCoordinate& end) {
    double lat1 = toRadians(start.latitude);
    double lon1 = toRadians(start.longitude);
    double lat2 = toRadians(end.latitude);
    double lon2 = toRadians(end.longitude);

    double dLon = lon2 - lon1;

    double y = std::sin(dLon) * std::cos(lat2);
    double x = std::cos(lat1) * std::sin(lat2) -
        std::sin(lat1) * std::cos(lat2) * std::cos(dLon);

    double brngRad = std::atan2(y, x);
    double brngDeg = toDegrees(brngRad);

    // 결과를 0-360 범위로 정규화
    return std::fmod(brngDeg + 360.0, 360.0);
}