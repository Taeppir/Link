cmake_minimum_required(VERSION 3.15)

# vcpkg toolchain 설정
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
endif()

project(ShipRouting VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================
# 날씨 데이터 경로 설정
# ============================================
set(WEATHER_DATA_PATH "D:/Capstone_Design/WeatherData_DSME_Format" 
    CACHE PATH "Weather data directory path")

message(STATUS "Weather Data Path: ${WEATHER_DATA_PATH}")

# ============================================
# GDAL 설정
# ============================================
set(GDAL_INCLUDE_DIR "C:/vcpkg/installed/x64-windows/include")
set(GDAL_LIBRARY "C:/vcpkg/installed/x64-windows/lib/gdal.lib")
find_package(GDAL QUIET)

message(STATUS "GDAL Include: ${GDAL_INCLUDE_DIR}")
message(STATUS "GDAL Library: ${GDAL_LIBRARY}")

# ============================================
# 1. types 라이브러리
# ============================================
add_library(types STATIC
    types/grid_types.cpp
    types/geo_types.cpp
)
target_include_directories(types PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# ============================================
# 2. utils 라이브러리
# ============================================
add_library(utils STATIC
    utils/dll_loader.cpp
    utils/fuel_calculator.cpp
    utils/geo_calculations.cpp
    utils/weather_interpolation.cpp
)
target_include_directories(utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(utils PUBLIC types)

# ============================================
# 3. data_loading 라이브러리
# ============================================
add_library(data_loading STATIC
    data_loading/gebco_loader.cpp
    data_loading/gshhs_loader.cpp
    data_loading/grid_builder.cpp
    data_loading/weather_loader.cpp
)
target_include_directories(data_loading PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GDAL_INCLUDE_DIR}
)
target_link_libraries(data_loading PUBLIC 
    types 
    ${GDAL_LIBRARY}
)

# ============================================
# 4. route_analysis 라이브러리
# ============================================
add_library(route_analysis STATIC
    route_analysis/waypoint_snapper.cpp
)
target_include_directories(route_analysis PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(route_analysis PUBLIC 
    types
    utils
)

# ============================================
# 5. pathfinding 라이브러리
# ============================================
add_library(pathfinding STATIC
    pathfinding/path_utils.cpp
    pathfinding/a_star_engine.cpp
    pathfinding/shortest_planner.cpp
    pathfinding/optimized_planner.cpp
)
target_include_directories(pathfinding PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(pathfinding PUBLIC 
    types
    utils
)

# ============================================
# 6. api 라이브러리 (ShipRouter)
# ============================================
add_library(ship_routing_api STATIC
    api/ship_router.cpp
)
target_include_directories(ship_routing_api PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ship_routing_api PUBLIC 
    types
    utils
    data_loading
    route_analysis
    pathfinding
)

# ============================================
# 테스트 실행 파일 빌드
# ============================================

# Test 1: Grid Builder & Waypoint Snapper 테스트
add_executable(test_grid_snapper
    test/test_grid_snapper.cpp
)
target_link_libraries(test_grid_snapper PRIVATE
    data_loading
    route_analysis
    types
    utils
)

# 디버깅 시 환경변수 설정 (Visual Studio용)
set_target_properties(test_grid_snapper PROPERTIES
    VS_DEBUGGER_ENVIRONMENT "WEATHER_DATA_PATH=${WEATHER_DATA_PATH}"
)

# Test 2: ShipRouter 전체 통합 테스트
add_executable(test_ship_router
    test/test_ship_router.cpp
)
target_link_libraries(test_ship_router PRIVATE
    ship_routing_api
    pathfinding
    data_loading
    route_analysis
    types
    utils
)

# 디버깅 시 환경변수 설정 (Visual Studio용)
set_target_properties(test_ship_router PROPERTIES
    VS_DEBUGGER_ENVIRONMENT "WEATHER_DATA_PATH=${WEATHER_DATA_PATH}"
)

# ============================================
# 빌드 정보 출력
# ============================================
message(STATUS "================================")
message(STATUS "Build Configuration:")
message(STATUS "  types: 2 files")
message(STATUS "  utils: 4 files")
message(STATUS "  data_loading: 4 files")
message(STATUS "  route_analysis: 1 file")
message(STATUS "  pathfinding: 4 files")
message(STATUS "  api: 1 file (ship_router)")
message(STATUS "")
message(STATUS "Test Targets:")
message(STATUS "  test_grid_snapper    - Grid & Snapping test")
message(STATUS "  test_ship_router     - Full integration test")
message(STATUS "  Environment: WEATHER_DATA_PATH=${WEATHER_DATA_PATH}")
message(STATUS "================================")