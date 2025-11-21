#pragma once
#include "../types/weather_types.h"
#include <map>

// ===== 기존 parse_weather_data.h 내용 100% 그대로 =====

Weather getWeatherAtCoordinate(
    const std::map<std::string, WeatherDataInput>& all_weather_data,
    unsigned int time,
    double lat,
    double lon
);