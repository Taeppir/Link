#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include "../../include/weather_types.h"

bool readWeatherFile(const std::string& filePath, WeatherDataInput& weatherData) {
    FILE* fp = nullptr;
    errno_t err = fopen_s(&fp, filePath.c_str(), "rb");

    if (err || !fp) {
        std::cerr << "File not found: " << filePath << std::endl;
        return false;
    }

    fread(&weatherData.iStartTime, sizeof(unsigned int), 1, fp);
    fread(&weatherData.iNumTime, sizeof(unsigned int), 1, fp);
    fread(&weatherData.iTimeBin, sizeof(unsigned int), 1, fp);
    fread(&weatherData.StartLon, sizeof(float), 1, fp);
    fread(&weatherData.iNumLon, sizeof(unsigned int), 1, fp);
    fread(&weatherData.LonBin, sizeof(float), 1, fp);
    fread(&weatherData.StartLat, sizeof(float), 1, fp);
    fread(&weatherData.iNumLat, sizeof(unsigned int), 1, fp);
    fread(&weatherData.LatBin, sizeof(float), 1, fp);
    
    size_t totalElements = static_cast<size_t>(weatherData.iNumTime) * weatherData.iNumLon * weatherData.iNumLat;
    weatherData.data.resize(totalElements);

    fread(weatherData.data.data(), sizeof(float), totalElements, fp);

    fclose(fp);
    return true;
}

std::map<std::string, WeatherDataInput> storeWeatherData() {
    std::string base_path;
    char* weather_data_path_env = nullptr;
    size_t len = 0;

    if (_dupenv_s(&weather_data_path_env, &len, "WEATHER_DATA_PATH") == 0 && weather_data_path_env != nullptr) {
        base_path = weather_data_path_env;
        free(weather_data_path_env);
    }
    else {
        std::cerr << "Error: The environment variable 'WEATHER_DATA_PATH' is not set or found." << std::endl;
    }

    std::vector<std::string> fileNames = {
        "WindDir.bin", "WindSpd.bin", "CurrDir.bin", "CurrSpd.bin",
        "WaveDir.bin", "WaveHgt.bin", "WavePrd.bin"
    };

    std::map<std::string, WeatherDataInput> all_weather_data;

    std::cout << "Loading weather data files..." << std::endl;

    for (const auto& fileName : fileNames) {
        WeatherDataInput weatherData;
        std::string fullPath = base_path + fileName;
        if (readWeatherFile(fullPath, weatherData)) {
            all_weather_data[fileName] = weatherData;
            std::cout << "Successfully read " << fileName << std::endl;
        }
    }

    std::cout << "\nAll files processed." << std::endl;

    return all_weather_data;
}