#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <nlohmann/json.hpp>
#include "JSON_maker.h"

using json = nlohmann::json;

// ==========================================================
// 헬퍼 함수
// ==========================================================

json MakePointFeature(const GeoCoordinate& pt, const std::string& name, const std::string& color) {
    json feature = json::object();
    feature["type"] = "Feature";
    feature["properties"] = {
        {"name", name},
        {"marker-color", color},
        {"marker-size", "medium"},
        {"marker-symbol", ""}
    };
    feature["geometry"] = {
        {"type", "Point"},
        {"coordinates", {pt.longitude, pt.latitude}}
    };
    return feature;
}

json MakeLineFeature(const std::vector<GeoCoordinate>& points, const std::string& color, double width, const std::string& dashArray = "") {
    json feature = json::object();
    feature["type"] = "Feature";
    feature["properties"] = {
        {"stroke", color},
        {"stroke-width", width},
        {"stroke-opacity", 1}
    };
    if (!dashArray.empty()) {
        feature["properties"]["stroke-dasharray"] = dashArray;
    }
    feature["geometry"] = {
        {"type", "LineString"},
        {"coordinates", json::array()}
    };
    for (const auto& p : points) {
        feature["geometry"]["coordinates"].push_back({ p.longitude, p.latitude });
    }
    return feature;
}

// ==========================================================
// 구현
// ==========================================================

// [Simple] 경로선 + 실제 경유지(스내핑된 좌표) 마커
void SaveGeoPointsToJson(
    const std::vector<GeoCoordinate>& path_points,
    const std::vector<SnappingInfo>& snapping_infos,
    const std::string& filename,
    const std::string& stroke_color
) {
    json feature_collection = json::object();
    feature_collection["type"] = "FeatureCollection";
    feature_collection["features"] = json::array();

    // 1. 메인 경로 선
    feature_collection["features"].push_back(MakeLineFeature(path_points, stroke_color, 4));

    // 2. 주요 경유지 마커 (스내핑된 실제 배가 가는 위치) - 색상은 경로색과 동일하게
    for (size_t i = 0; i < snapping_infos.size(); ++i) {
        std::string name = "WP " + std::to_string(i);
        if (i == 0) name = "Start";
        else if (i == snapping_infos.size() - 1) name = "End";

        // 스내핑된 좌표에 마커 찍기
        feature_collection["features"].push_back(
            MakePointFeature(snapping_infos[i].snapped, name, stroke_color)
        );
    }

    std::ofstream file(filename);
    if (file.is_open()) {
        file << feature_collection.dump(4);
        file.close();
    }
}

// [Debug] 원본(회색) <-> 스내핑(빨강) 연결선 + 메인 경로
void SaveRouteDebugJson(
    const std::vector<GeoCoordinate>& path_points,
    const std::vector<SnappingInfo>& snapping_infos,
    const std::string& filename,
    const std::string& main_path_color
) {
    json feature_collection = json::object();
    feature_collection["type"] = "FeatureCollection";
    feature_collection["features"] = json::array();

    // -------------------------------------------------------
    // 1. 모든 웨이포인트에 대해 (원본 <-> 스내핑) 시각화
    // -------------------------------------------------------
    std::string connector_color = "#808080"; // 회색

    for (size_t i = 0; i < snapping_infos.size(); ++i) {
        const auto& info = snapping_infos[i];
        
        std::string label_idx = std::to_string(i);
        if (i == 0) label_idx = "Start";
        else if (i == snapping_infos.size() - 1) label_idx = "End";

        // (1) 연결선 (원본 -> 스내핑) : 점선
        feature_collection["features"].push_back(
            MakeLineFeature({ info.original, info.snapped }, connector_color, 2, "5, 5")
        );

        // (2) 원본 마커 (회색)
        feature_collection["features"].push_back(
            MakePointFeature(info.original, "Original " + label_idx, "#808080")
        );

        // (3) 스내핑 마커 (빨강)
        feature_collection["features"].push_back(
            MakePointFeature(info.snapped, "Snapped " + label_idx, "#FF0000")
        );
    }

    // -------------------------------------------------------
    // 2. 메인 경로 그리기
    // -------------------------------------------------------
    // path_points는 이미 스내핑된 점들을 포함하거나 연결한 경로라고 가정
    feature_collection["features"].push_back(
        MakeLineFeature(path_points, main_path_color, 4)
    );

    // 파일 저장
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Cannot open JSON file: " << filename << "\n";
        return;
    }
    file << feature_collection.dump(4);
    file.close();
}