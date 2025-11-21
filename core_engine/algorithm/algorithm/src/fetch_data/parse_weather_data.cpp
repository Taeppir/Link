#include <cmath> 
#include <iostream>
#include <map>
#include <algorithm>
#include "../../include/read_weather_data.h"
#include "../../include/parse_weather_data.h"

static int get3dIndex(const WeatherDataInput& data, unsigned int time, double lat, double lon) {
    // 1) time index (second → hour → 3 hours bin)
    double elapsed_seconds = (double)time - data.iStartTime;
    if (elapsed_seconds < 0) return -1;
    double elapsed_hours = elapsed_seconds / 3600.0;
    int time_idx = (int)std::floor(elapsed_hours / (double)data.iTimeBin);
    int NT = (int)data.iNumTime;
    if (NT > 0) time_idx = (time_idx % NT + NT) % NT;

    // 2) lon normalize (StartLon 기준)
    double lon_rel = std::fmod(std::fmod(lon - data.StartLon, 360.0) + 360.0, 360.0);
    double lon_norm = data.StartLon + lon_rel;
    int lon_idx = (int)std::floor((lon_norm - data.StartLon) / data.LonBin + 1e-9);

    // 3) lat index 
    int lat_idx;
    if (data.LatBin > 0.0) {
        lat_idx = (int)std::floor((data.StartLat - lat) / data.LatBin + 1e-9);
    }
    else {
        lat_idx = (int)std::floor((lat - data.StartLat) / data.LatBin + 1e-9);
    }

    // 4) clamp
    lon_idx = std::clamp(lon_idx, 0, (int)data.iNumLon - 1);
    lat_idx = std::clamp(lat_idx, 0, (int)data.iNumLat - 1);

    // 5) linear index: [time][lon][lat]
    return ((time_idx * (int)data.iNumLon + lon_idx) * (int)data.iNumLat + lat_idx);
}


double getWeatherValue(const std::map<std::string, WeatherDataInput>& all_weather_data,
    const std::string& file_key,
    unsigned int time, double lat, double lon)
{
    auto it = all_weather_data.find(file_key);
    if (it == all_weather_data.end()) {
        return 0.0;
    }


    int index = get3dIndex(it->second, time, lat, lon);

    if (index != -1) {
        if (static_cast<size_t>(index) < it->second.data.size()) {

            double value = it->second.data[index];

			// Check for missing data values (e.g., -9999)
            // 특히 wave 값의 경우 -9999.0 아니면 양수(>0) 값으로만 나옴
            if (!std::isfinite(value) || value < -9000.0) {
                // Treat it as 0.0 (no weather effect)
                return 0.0; 
            }
            return value;
        }
        else {
            std::cerr << "  [Index Debug] FAILED calculated index " << index << " is out of data bounds." << std::endl;
            return 0.0;
        }
    }
    else {
        return 0.0;
    }
}

Weather getWeatherAtCoordinate(const std::map<std::string, WeatherDataInput>& all_weather_data, unsigned int time, double lat, double lon) {
    Weather conditions;

    const std::vector<std::string> file_keys = {
        "WindDir.bin", "WindSpd.bin", "CurrDir.bin", "CurrSpd.bin",
        "WaveDir.bin", "WaveHgt.bin", "WavePrd.bin"
    };



    for (const auto& key : file_keys) {
        double value = getWeatherValue(all_weather_data, key, time, lat, lon);

        if (key == "WindDir.bin") {
            conditions.windDir = value;
        }
        else if (key == "WindSpd.bin") {
            conditions.windSpd = value;
        }
        else if (key == "CurrDir.bin") {
            conditions.currDir = value;
        }
        else if (key == "CurrSpd.bin") {
            conditions.currSpd = value;
        }
        else if (key == "WaveDir.bin") {
            conditions.waveDir = value;
        }
        else if (key == "WaveHgt.bin") {
            conditions.waveHgt = value;
        }
        else if (key == "WavePrd.bin") {
            conditions.wavePrd = value;
        }
    }
    return conditions;

}