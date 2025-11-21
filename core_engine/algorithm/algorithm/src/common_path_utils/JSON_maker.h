#pragma once
#include <vector>
#include <string>
#include "../../include/common_types.h"

void SaveGeoPointsToJson(const std::vector<GeoCoordinate>& points,
    const std::string& filename,
    const std::string& color_hex);