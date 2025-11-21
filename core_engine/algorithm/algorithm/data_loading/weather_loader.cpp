#include "weather_loader.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <cstdlib>  // For getenv

namespace fs = std::filesystem;

WeatherLoader::WeatherLoader() {
}

WeatherLoader::~WeatherLoader() {
}

WeatherDataInput WeatherLoader::LoadBinaryFile(const std::string& filepath) {
    WeatherDataInput data;
    
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open weather file: " << filepath << std::endl;
        return data;
    }
    
    // 헤더 읽기
    file.read(reinterpret_cast<char*>(&data.iStartTime), sizeof(unsigned int));
    file.read(reinterpret_cast<char*>(&data.iNumTime), sizeof(unsigned int));
    file.read(reinterpret_cast<char*>(&data.iTimeBin), sizeof(unsigned int));
    file.read(reinterpret_cast<char*>(&data.StartLon), sizeof(float));
    file.read(reinterpret_cast<char*>(&data.iNumLon), sizeof(unsigned int));
    file.read(reinterpret_cast<char*>(&data.LonBin), sizeof(float));
    file.read(reinterpret_cast<char*>(&data.StartLat), sizeof(float));
    file.read(reinterpret_cast<char*>(&data.iNumLat), sizeof(unsigned int));
    file.read(reinterpret_cast<char*>(&data.LatBin), sizeof(float));
    
    // 데이터 크기 계산
    size_t totalSize = static_cast<size_t>(data.iNumTime) * 
                       static_cast<size_t>(data.iNumLon) * 
                       static_cast<size_t>(data.iNumLat);
    
    // 데이터 읽기
    data.data.resize(totalSize);
    file.read(reinterpret_cast<char*>(data.data.data()), totalSize * sizeof(float));
    
    file.close();
    
    std::cout << "  ✓ Loaded: " << fs::path(filepath).filename().string() << std::endl;
    std::cout << "    Time: " << data.iNumTime << " steps, Lat: " << data.iNumLat 
              << ", Lon: " << data.iNumLon << std::endl;
    
    return data;
}

std::map<std::string, WeatherDataInput> WeatherLoader::LoadWeatherData() {
    return LoadWeatherData("");  // Use environment variable
}

std::map<std::string, WeatherDataInput> WeatherLoader::LoadWeatherData(const std::string& directory) {
    std::map<std::string, WeatherDataInput> weatherData;
    
    // Determine weather data directory
    std::string weatherDir;
    
    if (!directory.empty()) {
        // Use provided directory
        weatherDir = directory;
        std::cout << "[WeatherLoader] Using provided directory: " << weatherDir << std::endl;
    } else {
        // Try to get from environment variable
        const char* envPath = std::getenv("WEATHER_DATA_PATH");
        if (envPath != nullptr) {
            weatherDir = envPath;
            std::cout << "[WeatherLoader] Using environment variable: " << weatherDir << std::endl;
        } else {
            // Fallback to default
            weatherDir = "./weather_data";
            std::cout << "[WeatherLoader] No environment variable, using default: " << weatherDir << std::endl;
        }
    }
    
    // Check if directory exists
    if (!fs::exists(weatherDir)) {
        std::cerr << "Warning: Weather data directory not found: " << weatherDir << std::endl;
        std::cerr << "Tip: Set WEATHER_DATA_PATH environment variable or provide directory path" << std::endl;
        std::cout << "Warning: No weather data loaded. Using zero weather conditions." << std::endl;
        return weatherData;
    }
    
    std::cout << "[WeatherLoader] Loading weather data from: " << weatherDir << std::endl;
    
    // Expected weather files
    const std::vector<std::string> weatherFiles = {
        "WindDir.bin",
        "WindSpd.bin",
        "CurrDir.bin",
        "CurrSpd.bin",
        "WaveDir.bin",
        "WaveHgt.bin",
        "WavePrd.bin"
    };
    
    // Load each file
    int loadedCount = 0;
    for (const auto& filename : weatherFiles) {
        fs::path filepath = fs::path(weatherDir) / filename;
        
        // 파일 존재 확인
        if (!fs::exists(filepath)) {
            std::cerr << "Warning: Weather file not found: " << filepath << std::endl;
            continue;
        }
        
        // 파일 로드
        WeatherDataInput data = LoadBinaryFile(filepath.string());
        
        // 로드 성공 시 맵에 추가
        if (!data.data.empty()) {
            weatherData[filename] = data;
            loadedCount++;
        }
    }
    
    if (weatherData.empty()) {
        std::cout << "Warning: No weather data loaded. Using zero weather conditions." << std::endl;
    } else {
        std::cout << "[WeatherLoader] Successfully loaded " << loadedCount << "/" 
                  << weatherFiles.size() << " weather files" << std::endl;
    }
    
    return weatherData;
}