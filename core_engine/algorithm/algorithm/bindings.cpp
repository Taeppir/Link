#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // std::vector, std::map 자동 변환 필수

// 프로젝트 헤더 파일 포함
#include "api/ship_router.h"
#include "results/route_results.h"
#include "types/geo_types.h"
#include "types/voyage_types.h"
#include "types/weather_types.h"

namespace py = pybind11;

// 모듈 정의
PYBIND11_MODULE(algorithm_module, m) {
    m.doc() = "Ship Routing Engine Python Binding";

    // ============================================================
    // 1. 기본 자료형 (GeoCoordinate)
    // ============================================================
    py::class_<GeoCoordinate>(m, "GeoCoordinate")
        .def(py::init<>())
        .def(py::init<double, double>(), py::arg("lat"), py::arg("lon"))
        .def_readwrite("latitude", &GeoCoordinate::latitude)
        .def_readwrite("longitude", &GeoCoordinate::longitude);

    // ============================================================
    // 2. 날씨 관련 데이터 구조
    // ============================================================
    
    py::class_<WeatherDataInput>(m, "WeatherDataInput")
        .def(py::init<>())
        .def_readwrite("iStartTime", &WeatherDataInput::iStartTime)
        .def_readwrite("iNumTime", &WeatherDataInput::iNumTime)
        .def_readwrite("iTimeBin", &WeatherDataInput::iTimeBin)
        .def_readwrite("StartLon", &WeatherDataInput::StartLon)
        .def_readwrite("iNumLon", &WeatherDataInput::iNumLon)
        .def_readwrite("LonBin", &WeatherDataInput::LonBin)
        .def_readwrite("StartLat", &WeatherDataInput::StartLat)
        .def_readwrite("iNumLat", &WeatherDataInput::iNumLat)
        .def_readwrite("LatBin", &WeatherDataInput::LatBin)
        .def_readwrite("data", &WeatherDataInput::data);

    py::class_<Weather>(m, "Weather")
        .def(py::init<>())
        .def_readwrite("windDir", &Weather::windDir)
        .def_readwrite("windSpd", &Weather::windSpd)
        .def_readwrite("currDir", &Weather::currDir)
        .def_readwrite("currSpd", &Weather::currSpd)
        .def_readwrite("waveDir", &Weather::waveDir)
        .def_readwrite("waveHgt", &Weather::waveHgt)
        .def_readwrite("wavePrd", &Weather::wavePrd);

    // ============================================================
    // 3. VoyageInfo (선박 상태 정보)
    // ============================================================
    py::class_<VoyageInfo>(m, "VoyageInfo")
        .def(py::init<>())
        .def_readwrite("heading", &VoyageInfo::heading)
        .def_readwrite("shipSpeed", &VoyageInfo::shipSpeed)
        .def_readwrite("draft", &VoyageInfo::draft)
        .def_readwrite("trim", &VoyageInfo::trim);

    // ============================================================
    // 4. VoyageConfig (항해 설정) - 중요!
    // ============================================================
    py::class_<VoyageConfig>(m, "VoyageConfig")
        .def(py::init<>())
        // Python snake_case -> C++ camelCase 매핑
        .def_readwrite("ship_speed_mps", &VoyageConfig::shipSpeedMps)
        .def_readwrite("draft_m", &VoyageConfig::draftM)
        .def_readwrite("trim_m", &VoyageConfig::trimM)
        .def_readwrite("grid_cell_size_km", &VoyageConfig::gridCellSizeKm)
        .def_readwrite("grid_margin_cells", &VoyageConfig::gridMarginCells)
        .def_readwrite("max_snap_radius_km", &VoyageConfig::maxSnapRadiusKm)
        .def_readwrite("start_time_unix", &VoyageConfig::startTimeUnix)
        .def_readwrite("calculate_shortest", &VoyageConfig::calculateShortest)
        .def_readwrite("calculate_optimized", &VoyageConfig::calculateOptimized);

    // ============================================================
    // 5. 경로 결과 데이터
    // ============================================================
    
    py::class_<PathSummary>(m, "RouteSummary")
        .def(py::init<>())
        .def_readwrite("total_distance_km", &PathSummary::total_distance_km)
        .def_readwrite("total_time_hours", &PathSummary::total_time_hours)
        .def_readwrite("total_fuel_kg", &PathSummary::total_fuel_kg)
        .def_readwrite("average_speed_mps", &PathSummary::average_speed_mps)
        .def_readwrite("average_fuel_rate_kg_per_hour", &PathSummary::average_fuel_rate_kg_per_hour);

    py::class_<PathPointDetail>(m, "PathPointDetail")
        .def(py::init<>())
        .def_readwrite("position", &PathPointDetail::position)
        .def_readwrite("cumulative_time_hours", &PathPointDetail::cumulative_time_hours)
        .def_readwrite("cumulative_distance_km", &PathPointDetail::cumulative_distance_km)
        .def_readwrite("cumulative_fuel_kg", &PathPointDetail::cumulative_fuel_kg)
        .def_readwrite("fuel_rate_kg_per_hour", &PathPointDetail::fuel_rate_kg_per_hour)
        .def_readwrite("speed_mps", &PathPointDetail::speed_mps)
        .def_readwrite("heading_degrees", &PathPointDetail::heading_degrees)
        .def_readwrite("weather", &PathPointDetail::weather);

    // 스냅핑 상태 Enum
    py::enum_<SnappingStatus>(m, "SnappingStatus")
        .value("ALREADY_NAVIGABLE", SnappingStatus::ALREADY_NAVIGABLE)
        .value("SNAPPED", SnappingStatus::SNAPPED)
        .value("FAILED", SnappingStatus::FAILED)
        .export_values();

    py::class_<SnappingInfo>(m, "SnappingInfo")
        .def(py::init<>())
        .def_readwrite("status", &SnappingInfo::status)
        .def_readwrite("original", &SnappingInfo::original)
        .def_readwrite("snapped", &SnappingInfo::snapped)
        .def_readwrite("was_snapped", &SnappingInfo::was_snapped)
        .def_readwrite("snapping_distance_km", &SnappingInfo::snapping_distance_km)
        .def_readwrite("failure_reason", &SnappingInfo::failure_reason);

    py::class_<SinglePathResult>(m, "SinglePathResult")
        .def(py::init<>())
        .def_readwrite("success", &SinglePathResult::success)
        .def_readwrite("error_message", &SinglePathResult::error_message)
        .def_readwrite("summary", &SinglePathResult::summary)
        .def_readwrite("path_details", &SinglePathResult::path_details);

    py::class_<VoyageResult>(m, "VoyageResult")
        .def(py::init<>())
        .def_readwrite("success", &VoyageResult::success)
        .def_readwrite("error_message", &VoyageResult::error_message)
        .def_readwrite("snapping_info", &VoyageResult::snapping_info)
        .def_readwrite("shortest_path", &VoyageResult::shortest_path)
        .def_readwrite("optimized_path", &VoyageResult::optimized_path);

    // ============================================================
    // 6. 메인 API 클래스 (ShipRouter)
    // ============================================================
    
    py::class_<ShipRouter>(m, "ShipRouter")
        .def(py::init<>())
        .def("initialize", &ShipRouter::Initialize, 
             py::arg("gebco_path"), 
             py::arg("gshhs_path"),
             "Initialize the router with GEBCO and GSHHS data files")
        .def("load_weather_data", &ShipRouter::LoadWeatherData, 
             py::arg("weather_dir"),
             "Load weather data from directory")
        .def("is_initialized", &ShipRouter::IsInitialized,
             "Check if router is initialized")
        // ✨ VoyageConfig를 직접 받도록 수정!
        .def("calculate_route", 
             py::overload_cast<const std::vector<GeoCoordinate>&, const VoyageConfig&>(
                 &ShipRouter::CalculateRoute),
             py::arg("waypoints"), 
             py::arg("config") = VoyageConfig(),
             "Calculate route through waypoints with given configuration");
}