#pragma once
#include "great_circle_route.h"

/**
 * @brief 거리와 속도로 소요 시간 계산
 * @param distance_km 거리 (km)
 * @param speed_mps 속도 (m/s)
 * @return 소요 시간 (hours)
 */
inline double timeCalculator(double distance_km, double speed_mps) {
    double distance_m = distance_km * 1000.0;
    double time_seconds = distance_m / speed_mps;
    double time_hours = time_seconds / 3600.0;
    return time_hours;
}

/**
 * @brief 두 지점 간 거리를 계산하고 소요 시간 반환
 * @param lat1 출발지 위도 (degrees)
 * @param lon1 출발지 경도 (degrees)
 * @param lat2 도착지 위도 (degrees)
 * @param lon2 도착지 경도 (degrees)
 * @param speed_mps 속도 (m/s)
 * @return 소요 시간 (hours)
 */
inline double gridTimeCalculator(double lat1, double lon1, double lat2, double lon2, double speed_mps) {
    double distance_km = greatCircleDistance(lat1, lon1, lat2, lon2);
    return timeCalculator(distance_km, speed_mps);
}