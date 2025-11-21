#pragma once

#include <string>
#include <vector>
#include <map>
#include "fetch_data.h"
#include "read_weather_data.h"
#include "common_types.h"

struct RouteAnalysisResult {
    bool success = false;
    std::string error_message;

    std::vector<GeoCoordinate> full_path_geo;

    double total_distance_km = 0.0;
    double total_time_hours = 0.0;
    double total_fuel_kg = 0.0;

    double optimal_distance_km = 0.0;
    double optimal_time_hours = 0.0;
    double optimal_fuel_kg = 0.0;
};

class ShipRouter {
public:
    ShipRouter();
    
    bool initialize(const std::string& gebco_path, const std::string& gshhs_path);

    RouteAnalysisResult calculateShortestPathAnalysis(
        const std::vector<GeoCoordinate>& waypoints_geo,
        double ship_speed_mps
    );

private:
    FetchData m_fetcher;
    std::map<std::string, WeatherDataInput> m_weather_data;
    bool m_is_initialized;
};
