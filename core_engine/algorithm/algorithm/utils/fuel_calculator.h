#pragma once
#include "../types/voyage_types.h"
#include "../types/weather_types.h"
#include "dll_loader.h"  // ← ShipOutput calculateFuelConsumption() 선언 포함
#include <map>

// ===== ShipDynamicsAPI.h는 dll_loader.h에서 include하므로 제거 가능 =====

// 날씨 데이터를 고려한 연료 계산
double fuelCalculator(
    unsigned int time, 
    double lat, 
    double lon, 
    const std::map<std::string, WeatherDataInput>& all_weather_data, 
    VoyageInfo& voyageInfo
);

// 날씨 데이터 없이 이상적인 연료 계산 (휴리스틱용)
double fuelCalculator_zero(
    unsigned int time, 
    double lat, 
    double lon, 
    VoyageInfo& voyageInfo
);