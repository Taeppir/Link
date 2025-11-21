#pragma once

#include "weather_types.h"

Weather getWeatherAtCoordinate(const std::map<std::string, WeatherDataInput>& all_weather_data, unsigned int time, double lat, double lon);