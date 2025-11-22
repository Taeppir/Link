#pragma once
#include "weather_types.h"
#include <string>

struct VoyageInfo {
    double heading = 0.0;     // degrees
    double shipSpeed = 0.0;   // m/s
    double draft = 0.0;       // meters
    double trim = 0.0;        // meters
};

struct VoyageConfig {
    // 선박 기본 정보
    double shipSpeedMps = 8.0;
    double draftM = 10.0;
    double trimM = 0.0;
    
    // 시뮬레이션 설정
    unsigned int startTimeUnix = 0;
    
    // 그리드 설정
    double gridCellSizeKm = 5.0;
    int gridMarginCells = 20;
    
    // 스냅핑 설정
    double maxSnapRadiusKm = 50.0;
    
    // 계산 옵션
    bool calculateShortest = true;
    bool calculateOptimized = true;

    std::string output_path = "";
};