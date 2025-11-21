#pragma once

#include "weather_types.h"
#include "ShipDynamicsAPI.h" 

double fuelCalculator(unsigned int time, double lat, double lon, const std::map<std::string, WeatherDataInput>& all_weather_data, VoyageInfo& voyageInfo);

double fuelCalculator_zero(unsigned int time, double lat, double lon, VoyageInfo& voyageInfo);

ShipOutput calculateFuelConsumption(const ShipInput& inputData);