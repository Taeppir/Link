#pragma once

// ===== VoyageInfo는 weather_types.h에 있으므로 include =====
#include "weather_types.h"

// ===== API 설정 구조체 (새로 추가) =====

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
};