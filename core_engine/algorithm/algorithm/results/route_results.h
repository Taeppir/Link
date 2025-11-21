// core_engine/algorithm/results/route_results.h
#pragma once

#include <vector>
#include <string>
#include "../types/geo_types.h"
#include "../types/weather_types.h"

// ============================================================
// 각 좌표점의 상세 정보
// ============================================================
struct PathPointDetail {
    GeoCoordinate position;           // 위경도 좌표
    
    // 누적 값
    double cumulative_time_hours;     // 누적 시간 (hour)
    double cumulative_distance_km;    // 누적 거리 (km)
    double cumulative_fuel_kg;        // 누적 연료소모량 (kg)
    
    // 현재 segment의 값
    double fuel_rate_kg_per_hour;     // 연료소모율 (kg/h)
    double speed_mps;                 // 속도 (m/s)
    double heading_degrees;           // 방향각 (degree)
    
    // 날씨 데이터
    Weather weather;              // 풍속, 풍향, 파고 등
    
    PathPointDetail()
        : cumulative_time_hours(0.0)
        , cumulative_distance_km(0.0)
        , cumulative_fuel_kg(0.0)
        , fuel_rate_kg_per_hour(0.0)
        , speed_mps(0.0)
        , heading_degrees(0.0)
    {}
};

// ============================================================
// 경로 요약 정보
// ============================================================
struct PathSummary {
    double total_distance_km;         // 총 거리 (km)
    double total_time_hours;          // 총 시간 (hour)
    double total_fuel_kg;             // 총 연료소모량 (kg)
    double average_speed_mps;         // 평균 속도 (m/s)
    double average_fuel_rate_kg_per_hour;  // 평균 연료소모율 (kg/h)
    
    PathSummary()
        : total_distance_km(0.0)
        , total_time_hours(0.0)
        , total_fuel_kg(0.0)
        , average_speed_mps(0.0)
        , average_fuel_rate_kg_per_hour(0.0)
    {}
};

// ============================================================
// 좌표 스냅핑 상태
// ============================================================
enum class SnappingStatus {
    ALREADY_NAVIGABLE,  // 원래 위치가 이미 항해 가능
    SNAPPED,            // 항해 가능한 위치로 스냅 성공
    FAILED              // 반경 내 항해 가능 위치 없음
};


// ============================================================
// 좌표 스내핑 정보
// ============================================================
struct SnappingInfo {
    SnappingStatus status;            // 스냅핑 상태
    GeoCoordinate original;           // 원본 좌표
    GeoCoordinate snapped;            // 스냅된 좌표
    bool was_snapped;                 // 스냅 여부
    double snapping_distance_km;      // 스냅 거리 (km)
    std::string failure_reason;       // 실패 이유 (실패시)
    
    SnappingInfo()
        : was_snapped(false)
        , snapping_distance_km(0.0)
    {}

    bool IsSuccess() const { 
        return status != SnappingStatus::FAILED; 
    }
    
    bool WasSnapped() const { 
        return status == SnappingStatus::SNAPPED; 
    }
    
    bool IsAlreadyNavigable() const { 
        return status == SnappingStatus::ALREADY_NAVIGABLE; 
    }
};

// ============================================================
// 단일 경로 결과 (최단 경로 또는 최적 경로)
// ============================================================
struct SinglePathResult {
    bool success;                     // 성공 여부
    std::string error_message;        // 에러 메시지
    
    PathSummary summary;             // 경로 요약
    std::vector<PathPointDetail> path_details;  // 각 좌표의 상세 정보
    
    SinglePathResult()
        : success(false)
    {}
};

// ============================================================
// 최종 항해 최적화 결과 (Python으로 전달)
// ============================================================
struct VoyageResult {
    bool success;                     // 전체 성공 여부
    std::string error_message;        // 에러 메시지
    
    // 입력 좌표 스내핑 정보
    std::vector<SnappingInfo> snapping_info;
    
    // 최단 경로 결과
    SinglePathResult shortest_path;
    
    // 최적 경로 결과
    SinglePathResult optimized_path;
    
    VoyageResult()
        : success(false)
    {}
};
