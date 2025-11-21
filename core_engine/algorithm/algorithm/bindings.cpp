//#include <pybind11/pybind11.h>
//#include <pybind11/stl.h> 
//#include "include/ship_router.h" 
//#include "include/common_types.h"
//
//namespace py = pybind11;
//
//PYBIND11_MODULE(algorithm_module, m) {
//    m.doc() = "Ship Route Optimization Module (C++)";
//
//    py::class_<GeoCoordinate>(m, "GeoCoordinate")
//        .def(py::init<>())
//        .def(py::init<double, double>(), py::arg("latitude"), py::arg("longitude"))
//        .def_readwrite("latitude", &GeoCoordinate::latitude)
//        .def_readwrite("longitude", &GeoCoordinate::longitude)
//        .def("__repr__", 
//            [](const GeoCoordinate& a) {
//                return "<GeoCoordinate(lat=" + std::to_string(a.latitude) +
//                    ", lon=" + std::to_string(a.longitude) + ")>";
//            }
//        );
//
//    py::class_<RouteAnalysisResult>(m, "RouteAnalysisResult")
//        .def(py::init<>())
//        .def_readonly("success", &RouteAnalysisResult::success)
//        .def_readonly("error_message", &RouteAnalysisResult::error_message)
//        .def_readonly("full_path_geo", &RouteAnalysisResult::full_path_geo)
//        .def_readonly("total_distance_km", &RouteAnalysisResult::total_distance_km)
//        .def_readonly("total_time_hours", &RouteAnalysisResult::total_time_hours)
//        .def_readonly("total_fuel_kg", &RouteAnalysisResult::total_fuel_kg)
//        .def_readonly("optimal_distance_km", &RouteAnalysisResult::optimal_distance_km)
//        .def_readonly("optimal_time_hours", &RouteAnalysisResult::optimal_time_hours)
//        .def_readonly("optimal_fuel_kg", &RouteAnalysisResult::optimal_fuel_kg);
//    py::class_<ShipRouter>(m, "ShipRouter")
//        .def(py::init<>()) // ������
//        .def("initialize", &ShipRouter::initialize,
//            "Loads all geographic and weather data",
//            py::arg("gebco_path"), py::arg("gshhs_path"))
//        .def("calculate_shortest_path_analysis", &ShipRouter::calculateShortestPathAnalysis,
//            "Calculates shortest A* path and its fuel consumption",
//            py::arg("waypoints_geo"), py::arg("ship_speed_mps"));
//}
