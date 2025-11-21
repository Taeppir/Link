#pragma once
#include "../types/weather_types.h"
#include <map>
#include <string>

// ===== 기존 read_weather_data.h를 클래스로 래핑 =====

class WeatherLoader {
public:
    WeatherLoader();
    ~WeatherLoader();
    
    // 날씨 데이터 로드 (기존 storeWeatherData 함수)
    std::map<std::string, WeatherDataInput> LoadWeatherData();
    
private:
    // 내부 헬퍼 함수들 (필요시)
    WeatherDataInput LoadBinaryFile(const std::string& filepath);
};