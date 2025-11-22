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
# Pybind11 경로 직접 지정
# ============================================
set(pybind11_DIR "C:/vcpkg/installed/x64-windows/share/pybind11")

find_package(pybind11 CONFIG REQUIRED)
message(STATUS "Pybind11 Found: ${pybind11_FOUND}")
message(STATUS "Pybind11 DIR: ${pybind11_DIR}")

# ============================================
# 데이터 경로 자동 설정 (통합 구조)
# ============================================
# 모든 데이터: core_engine/algorithm/algorithm/data/

set(DATA_DIR "${CMAKE_SOURCE_DIR}/data" CACHE PATH "Data directory")

# 1. GEBCO/GSHHS 경로
set(GEBCO_PATH "${DATA_DIR}/gebco/GEBCO_2024_sub_ice_topo.nc" 
    CACHE FILEPATH "GEBCO file path")
set(GSHHS_PATH "${DATA_DIR}/gshhs/GSHHS_i_L1.shp" 
    CACHE FILEPATH "GSHHS file path")

# 2. 날씨 데이터 경로 (우선순위: 환경변수 > data/weather/)
if(NOT DEFINED WEATHER_DATA_PATH)
    if(DEFINED ENV{WEATHER_DATA_PATH})
        set(WEATHER_DATA_PATH "$ENV{WEATHER_DATA_PATH}" 
            CACHE PATH "Weather data directory path")
        message(STATUS "Weather data path from environment: ${WEATHER_DATA_PATH}")
    else()
        set(WEATHER_DATA_PATH "${DATA_DIR}/weather" 
            CACHE PATH "Weather data directory path")
        message(STATUS "Weather data path (default): ${WEATHER_DATA_PATH}")
    endif()
endif()

# 3. DLL 경로
set(DLL_SOURCE_DIR "${DATA_DIR}/dll" CACHE PATH "DLL source directory")


message(STATUS "================================")
message(STATUS "Data Paths (all in ${DATA_DIR}):")
message(STATUS "  GEBCO:        ${GEBCO_PATH}")
message(STATUS "  GSHHS:        ${GSHHS_PATH}")
message(STATUS "  Weather Data: ${WEATHER_DATA_PATH}")
message(STATUS "  DLL Source:   ${DLL_SOURCE_DIR}")
message(STATUS "================================")

# ============================================
# GDAL 설정
# ============================================
set(GDAL_INCLUDE_DIR "C:/vcpkg/installed/x64-windows/include")
set(GDAL_LIBRARY "C:/vcpkg/installed/x64-windows/lib/gdal.lib")
find_package(GDAL QUIET)

message(STATUS "GDAL Include: ${GDAL_INCLUDE_DIR}")
message(STATUS "GDAL Library: ${GDAL_LIBRARY}")

# ============================================
# DLL 자동 복사 함수
# ============================================
function(copy_dll_to_target target_name)
    # ShipDynamics.dll 찾기
    set(DLL_FILE "${DLL_SOURCE_DIR}/ShipDynamics.dll")
    
    if(EXISTS "${DLL_FILE}")
        # 빌드 후 DLL을 실행 파일과 같은 디렉토리로 복사
        add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${DLL_FILE}"
                "$<TARGET_FILE_DIR:${target_name}>/ShipDynamics.dll"
            COMMENT "Copying ShipDynamics.dll to ${target_name} output directory"
        )
        message(STATUS "  [DLL] Will copy ShipDynamics.dll for ${target_name}")
    else()
        message(WARNING "  [DLL] ShipDynamics.dll not found at: ${DLL_FILE}")
    endif()
endfunction()

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
# 7. Python Binding Module (algorithm_module)
# ============================================
pybind11_add_module(algorithm_module
    bindings.cpp
)

# 모듈이 기존 엔진 기능들을 사용할 수 있도록 라이브러리를 링크
target_link_libraries(algorithm_module PRIVATE
    ship_routing_api
    pathfinding
    route_analysis
    data_loading
    utils
    types
)

# Python 모듈에도 DLL 복사
copy_dll_to_target(algorithm_module)

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

# 환경변수 및 DLL 설정
set_target_properties(test_grid_snapper PROPERTIES
    VS_DEBUGGER_ENVIRONMENT "WEATHER_DATA_PATH=${WEATHER_DATA_PATH}"
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"  # ✨ 작업 디렉토리 설정
)
copy_dll_to_target(test_grid_snapper)

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

# 환경변수 및 DLL 설정
set_target_properties(test_ship_router PROPERTIES
    VS_DEBUGGER_ENVIRONMENT "WEATHER_DATA_PATH=${WEATHER_DATA_PATH}"
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"  # ✨ 작업 디렉토리 설정
)
copy_dll_to_target(test_ship_router)

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
message(STATUS "  [PYTHON] algorithm_module: bindings.cpp") 
message(STATUS "")
message(STATUS "Test Targets:")
message(STATUS "  test_grid_snapper     - Grid & Snapping test")
message(STATUS "  test_ship_router      - Full integration test")
message(STATUS "  algorithm_module      - Python binding (.pyd)")
message(STATUS "")
message(STATUS "Auto Settings:")
message(STATUS "  Working Directory: ${CMAKE_SOURCE_DIR}")
message(STATUS "  DLL Auto-Copy: Enabled")
message(STATUS "  Environment: WEATHER_DATA_PATH=${WEATHER_DATA_PATH}")
message(STATUS "================================")