#include "weather_loader.h"
#include <fstream>
#include <iostream>
#include <filesystem>

// ===== 기존 read_weather_data.cpp 로직을 클래스 메서드로 변환 =====

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
    
    std::cout << "Loaded weather file: " << filepath << std::endl;
    std::cout << "  Time: " << data.iNumTime << " steps, Lat: " << data.iNumLat 
              << ", Lon: " << data.iNumLon << std::endl;
    
    return data;
}

std::map<std::string, WeatherDataInput> WeatherLoader::LoadWeatherData() {
    std::map<std::string, WeatherDataInput> weatherData;
    
    // 날씨 데이터 파일 경로 (실제 경로로 수정 필요)
    const std::string weatherDir = "./weather_data/";  // 또는 설정 파일에서 읽기
    
    const std::vector<std::string> weatherFiles = {
        "WindDir.bin",
        "WindSpd.bin",
        "CurrDir.bin",
        "CurrSpd.bin",
        "WaveDir.bin",
        "WaveHgt.bin",
        "WavePrd.bin"
    };
    
    for (const auto& filename : weatherFiles) {
        std::string filepath = weatherDir + filename;
        
        // 파일 존재 확인
        if (!std::filesystem::exists(filepath)) {
            std::cerr << "Warning: Weather file not found: " << filepath << std::endl;
            continue;
        }
        
        // 파일 로드
        WeatherDataInput data = LoadBinaryFile(filepath);
        
        // 로드 성공 시 맵에 추가
        if (!data.data.empty()) {
            weatherData[filename] = data;
        }
    }
    
    if (weatherData.empty()) {
        std::cout << "Warning: No weather data loaded. Using zero weather conditions." << std::endl;
    } else {
        std::cout << "Successfully loaded " << weatherData.size() << " weather files." << std::endl;
    }
    
    return weatherData;
}