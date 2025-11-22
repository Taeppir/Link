#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // std::vector, std::map 자동 변환 필수

// 프로젝트 헤더 파일 포함
#include "api/ship_router.h"
#include "results/route_results.h"
#include "types/geo_types.h"
#include "types/voyage_types.h"
#include "types/weather_types.h" 
// grid_types.h 등 필요한 헤더가 더 있다면 포함

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
    // 2. 날씨 관련 데이터 구조 (Weather, WeatherDataInput)
    // ============================================================
    
    // WeatherDataInput (바이너리 파일 메타데이터 및 데이터)
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
        .def_readwrite("data", &WeatherDataInput::data); // std::vector<float> -> Python list[float]

    // Weather (개별 지점 날씨 상세)
    py::class_<Weather>(m, "Weather")
        .def(py::init<>())
        .def_readwrite("windDir", &Weather::windDir) // degrees
        .def_readwrite("windSpd", &Weather::windSpd) // m/s
        .def_readwrite("currDir", &Weather::currDir) // degrees
        .def_readwrite("currSpd", &Weather::currSpd) // m/s
        .def_readwrite("waveDir", &Weather::waveDir) // degrees
        .def_readwrite("waveHgt", &Weather::waveHgt) // meters
        .def_readwrite("wavePrd", &Weather::wavePrd); // seconds

    // ============================================================
    // 3. 선박 및 운항 정보 (VoyageInfo)
    // ============================================================
    py::class_<VoyageInfo>(m, "VoyageInfo")
        .def(py::init<>())
        .def_readwrite("heading", &VoyageInfo::heading)
        .def_readwrite("shipSpeed", &VoyageInfo::shipSpeed)
        .def_readwrite("draft", &VoyageInfo::draft)
        .def_readwrite("trim", &VoyageInfo::trim);

    // 만약 API에서 VoyageConfig라는 이름을 별도로 사용한다면 아래와 같이 별칭을 주거나 바인딩합니다.
    // 여기서는 VoyageInfo를 VoyageConfig로 사용하는 경우를 가정하여 처리합니다.
    // typedef VoyageInfo VoyageConfig; // (헤더에 정의되어 있다고 가정)

    // ============================================================
    // 4. 경로 결과 데이터 (PathSummary, PathPointDetail 등)
    // ============================================================
    
    py::class_<PathSummary>(m, "PathSummary")
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
        .def_readwrite("weather", &PathPointDetail::weather); // 위에서 정의한 Weather 구조체 사용

    // 스내핑 관련 Enum 및 Info
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

    // 결과 컨테이너
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
    // 5. 메인 API 클래스 (ShipRouter)
    // ============================================================
    
    // VoyageConfig가 VoyageInfo와 같다면, 별도 바인딩 없이 인자 타입만 맞추면 됩니다.
    // 만약 헤더에 `using VoyageConfig = VoyageInfo;`가 있다면 VoyageInfo 바인딩으로 충분합니다.
    // 여기서는 ShipRouter::CalculateRoute가 VoyageInfo(또는 Config)를 인자로 받는다고 가정합니다.

    py::class_<ShipRouter>(m, "ShipRouter")
        .def(py::init<>())
        .def("initialize", &ShipRouter::Initialize, 
             py::arg("gebco_path"), 
             py::arg("gshhs_path"))
        .def("load_weather_data", &ShipRouter::LoadWeatherData, 
             py::arg("weather_dir"))
        // Python에서 calculate_route(waypoints, config=VoyageInfo()) 형태로 호출 가능
        .def("calculate_route", [](ShipRouter& self, const std::vector<GeoCoordinate>& waypoints, const VoyageInfo& config) {
            // C++ 쪽 CalculateRoute가 VoyageConfig 타입을 받는다면 내부에서 캐스팅 필요할 수 있음
            // VoyageTypes.h에서 struct VoyageConfig : public VoyageInfo {}; 형태이거나 using이면 그대로 전달
             return self.CalculateRoute(waypoints, (const VoyageConfig&)config); 
        }, py::arg("waypoints"), py::arg("config") = VoyageInfo());
}