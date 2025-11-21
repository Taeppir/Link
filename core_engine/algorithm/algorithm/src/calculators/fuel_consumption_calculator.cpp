#include <iostream>
#include <iomanip>
#include "../../include/parse_weather_data.h"
#include "../../include/fuel_consumption_calculator.h"

double fuelCalculator(unsigned int time, double lat, double lon, const std::map<std::string, WeatherDataInput>& all_weather_data, VoyageInfo& voyageInfo) {

    ShipInput input;
    std::memset(&input, 0, sizeof(ShipInput));

    ShipOutput result;
    std::memset(&result, 0, sizeof(ShipOutput));

    Weather current_weather = getWeatherAtCoordinate(all_weather_data, time, lat, lon);

    input.windDirectionDeg = current_weather.windDir;
    input.windSpeed = current_weather.windSpd;
    input.currentDirectionDeg = current_weather.currDir;
    input.currentSpeed = current_weather.currSpd;
    input.waveHeight = current_weather.waveHgt;
    input.waveDirectionDeg = current_weather.waveDir;
    input.wavePeriod = current_weather.wavePrd;

    input.heading = voyageInfo.heading;
    input.shipSpeed = voyageInfo.shipSpeed;
    input.draft = voyageInfo.draft;
    input.trim = voyageInfo.trim;
    input.waps_type = 1;

    try {
        result = calculateFuelConsumption(input);
    }
    catch (const std::exception& e) {
        std::cerr << "\n[ERROR] DLL call failed!" << std::endl;
        std::cerr << "Error: " << e.what() << std::endl;
        std::cerr << "Location: time=" << time << " lat=" << lat << " lon=" << lon << std::endl;
        std::cerr << "Weather: wind=" << input.windSpeed
            << " wave=" << input.waveHeight << std::endl;
        throw;
    }
    return result.fuelConsumption;
}

double fuelCalculator_zero(unsigned int time, double lat, double lon, VoyageInfo& voyageInfo) {
    ShipInput input = {};
    ShipOutput result = {};

    input.windDirectionDeg = 0;
    input.windSpeed = 0;
    input.currentDirectionDeg = 0;
    input.currentSpeed = 0;
    input.waveHeight = 0;
    input.waveDirectionDeg = 0;
    input.wavePeriod = 0;

    input.heading = voyageInfo.heading;
    input.shipSpeed = voyageInfo.shipSpeed;
    input.draft = voyageInfo.draft;
    input.trim = voyageInfo.trim;
    input.waps_type = 1;

    try {
        result = calculateFuelConsumption(input);
    }
    catch (const std::exception& e) {
        std::cerr << "[ERROR] DLL call failed: " << e.what() << std::endl;
        return 0.0;
    }
    return result.fuelConsumption;
}