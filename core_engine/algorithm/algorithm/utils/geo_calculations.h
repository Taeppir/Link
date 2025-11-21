#pragma once
#include "../types/geo_types.h"
#include <vector>

// ===== 기존 great_circle_route.h + geo_utils.h 병합 =====

inline constexpr double PI = 3.14159265358979323846;

// 대권거리 계산 (Haversine formula)
double greatCircleDistance(double lat1, double lon1, double lat2, double lon2);

// 대권 경로상의 중간 지점들 생성
std::vector<GeoCoordinate> generateGreatCirclePoints(
    GeoCoordinate start, 
    GeoCoordinate end, 
    int num_points = 50
);

// 방위각 계산 (bearing)
double calculateBearing(const GeoCoordinate& from, const GeoCoordinate& to);