#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <nlohmann/json.hpp>
#include "JSON_maker.h"

using json = nlohmann::json;

void SaveGeoPointsToJson(
    const std::vector<GeoCoordinate>& points,
    const std::string& filename,
    const std::string& stroke_color
) {
    json feature = json::object();

    feature["type"] = "Feature";

    feature["properties"] = {
        {"stroke", stroke_color},
        {"stroke-width", 3}
    };

    feature["geometry"] = json::object();
    feature["geometry"]["type"] = "LineString";

    for (const auto& p : points) {
        feature["geometry"]["coordinates"].push_back({ p.longitude, p.latitude });
    }

    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Cannot open JSON file: " << filename << "\n";
        return;
    }

    file << feature.dump(4);
    file.close();
}
