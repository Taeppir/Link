#pragma once

// ===== 기존 time_calculator.h 내용 100% 그대로 =====

// 거리(km)와 속도(m/s)로 시간(hours) 계산
inline double timeCalculator(double distance_km, double speed_mps) {
    if (speed_mps <= 0.0) {
        return 0.0;
    }
    
    double distance_m = distance_km * 1000.0;
    double time_seconds = distance_m / speed_mps;
    double time_hours = time_seconds / 3600.0;
    
    return time_hours;
}