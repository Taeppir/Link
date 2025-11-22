#pragma once
#include <vector>
#include <string>
#include "../types/geo_types.h"
#include "../results/route_results.h"

// [Simple] 단순 경로 + 주요 경유지 마커
void SaveGeoPointsToJson(
    const std::vector<GeoCoordinate>& path_points,
    const std::vector<SnappingInfo>& snapping_infos,
    const std::string& filename,
    const std::string& color_hex
);

// [Debug] 상세 디버깅 (원본 <-> 스내핑 연결선 포함)
void SaveRouteDebugJson(
    const std::vector<GeoCoordinate>& path_points,
    const std::vector<SnappingInfo>& snapping_infos, 
    const std::string& filename,
    const std::string& main_path_color
);