cmake_minimum_required(VERSION 3.15)

# ============================================
# CRITICAL: 현재 활성화된 Python 자동 감지
# ============================================
# vcpkg의 Python을 찾지 않도록 설정
set(Python_FIND_REGISTRY NEVER)
set(Python_FIND_VIRTUALENV FIRST)  # 가상환경 우선
set(CMAKE_FIND_USE_PACKAGE_REGISTRY FALSE)

# PATH에서 python.exe 찾기 (conda 환경이 활성화되어 있으면 자동으로 찾음)
find_program(ACTIVE_PYTHON_EXECUTABLE 
    NAMES python python.exe
    PATHS ENV PATH
    NO_DEFAULT_PATH
    NO_CMAKE_FIND_ROOT_PATH
)

if(ACTIVE_PYTHON_EXECUTABLE)
    message(STATUS "🔍 발견된 Python: ${ACTIVE_PYTHON_EXECUTABLE}")
    
    # Python 버전 확인
    execute_process(
        COMMAND "${ACTIVE_PYTHON_EXECUTABLE}" --version
        OUTPUT_VARIABLE PYTHON_VERSION_STRING
        ERROR_VARIABLE PYTHON_VERSION_STRING
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_STRIP_TRAILING_WHITESPACE
    )
    message(STATUS "📌 Python 버전: ${PYTHON_VERSION_STRING}")
    
    # Python 3.9가 아니면 경고
    if(NOT PYTHON_VERSION_STRING MATCHES "3\\.9")
        message(WARNING "⚠️  Python 3.9가 아닙니다: ${PYTHON_VERSION_STRING}")
        message(WARNING "⚠️  'conda activate ship_env' 를 실행했는지 확인하세요!")
    endif()
    
    # Python 루트 디렉토리 추출 (python.exe의 상위 디렉토리)
    get_filename_component(PYTHON_ROOT_DIR "${ACTIVE_PYTHON_EXECUTABLE}" DIRECTORY)
    set(PYTHON_ROOT_DIR "${PYTHON_ROOT_DIR}" CACHE PATH "Python root directory" FORCE)
    message(STATUS "🔍 Python 루트: ${PYTHON_ROOT_DIR}")
    
    # Python 경로 강제 설정
    set(Python_ROOT_DIR "${PYTHON_ROOT_DIR}" CACHE PATH "Python root directory" FORCE)
    set(Python_EXECUTABLE "${ACTIVE_PYTHON_EXECUTABLE}" CACHE FILEPATH "Python executable" FORCE)
    set(Python3_EXECUTABLE "${ACTIVE_PYTHON_EXECUTABLE}" CACHE FILEPATH "Python3 executable" FORCE)
    
    # Python Development 경로 명시
    set(Python_INCLUDE_DIR "${PYTHON_ROOT_DIR}/include" CACHE PATH "Python include directory" FORCE)
    set(Python_LIBRARY "${PYTHON_ROOT_DIR}/libs/python39.lib" CACHE FILEPATH "Python library" FORCE)
    set(Python_INCLUDE_DIRS "${PYTHON_ROOT_DIR}/include" CACHE PATH "Python include directories" FORCE)
    set(Python_LIBRARIES "${PYTHON_ROOT_DIR}/libs/python39.lib" CACHE FILEPATH "Python libraries" FORCE)
    
    message(STATUS "📂 Python Include: ${Python_INCLUDE_DIR}")
    message(STATUS "📂 Python Library: ${Python_LIBRARY}")
else()
    message(FATAL_ERROR "❌ Python을 찾을 수 없습니다! 'conda activate ship_env' 를 실행하세요.")
endif()

# vcpkg toolchain 설정 (Python 설정 후에!)
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
endif()

project(ShipRouting VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# UTF-8 인코딩 설정 (인코딩 경고 제거)
if(MSVC)
    add_compile_options(/utf-8)
endif()

# ============================================
# Python 검증 및 확인
# ============================================
find_package(Python 3.9 EXACT REQUIRED COMPONENTS Interpreter)
find_package(Python 3.9 EXACT COMPONENTS Development)

message(STATUS "================================")
message(STATUS "Python Configuration:")
message(STATUS "  ✅ Python Version: ${Python_VERSION}")
message(STATUS "  ✅ Python Executable: ${Python_EXECUTABLE}")
if(Python_INCLUDE_DIRS)
    message(STATUS "  ✅ Python Include: ${Python_INCLUDE_DIRS}")
endif()
if(Python_LIBRARIES)
    message(STATUS "  ✅ Python Libraries: ${Python_LIBRARIES}")
endif()
message(STATUS "================================")

# pybind11이 필요로 하는 변수들 설정
if(NOT Python_INCLUDE_DIRS)
    set(Python_INCLUDE_DIRS "${PYTHON_ROOT_DIR}/include" CACHE PATH "Python include dirs" FORCE)
endif()
if(NOT Python_LIBRARIES)
    set(Python_LIBRARIES "${PYTHON_ROOT_DIR}/libs/python39.lib" CACHE FILEPATH "Python libs" FORCE)
endif()

set(PYBIND11_FINDPYTHON ON)
set(PYBIND11_PYTHON_VERSION "3.9")

# ============================================
# Pybind11 설정
# ============================================
set(pybind11_DIR "C:/vcpkg/installed/x64-windows/share/pybind11")

find_package(pybind11 CONFIG REQUIRED)
message(STATUS "Pybind11 Found: ${pybind11_FOUND}")
message(STATUS "Pybind11 DIR: ${pybind11_DIR}")

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
    # 프로젝트 루트의 data/dll/ShipDynamics.dll
    set(DLL_SOURCE "${CMAKE_SOURCE_DIR}/../../../data/dll/ShipDynamics.dll")
    
    if(EXISTS "${DLL_SOURCE}")
        # 빌드 디렉토리로 복사
        add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${DLL_SOURCE}"
                "$<TARGET_FILE_DIR:${target_name}>/ShipDynamics.dll"
            COMMENT "Copying ShipDynamics.dll to ${target_name} output directory"
        )
        message(STATUS "  [DLL] Will copy ShipDynamics.dll for ${target_name}")
    else()
        message(WARNING "  [DLL] ShipDynamics.dll not found at: ${DLL_SOURCE}")
        message(WARNING "  [DLL] Please place ShipDynamics.dll in: LINK/data/dll/")
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
pybind11_add_module(algorithm_module bindings.cpp)

target_link_libraries(algorithm_module PRIVATE
    ship_routing_api
    pathfinding
    route_analysis
    data_loading
    utils
    types
)

# .pyd 파일을 프로젝트 루트로 자동 복사
add_custom_command(TARGET algorithm_module POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:algorithm_module>"
        "${CMAKE_SOURCE_DIR}/../../../$<TARGET_FILE_NAME:algorithm_module>"
    COMMENT "Copying .pyd to project root: ${CMAKE_SOURCE_DIR}/../../../"
)

# Python 모듈에도 DLL 복사 (빌드 디렉토리용)
copy_dll_to_target(algorithm_module)

# ============================================
# 테스트 실행 파일 빌드 (선택사항)
# ============================================

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
copy_dll_to_target(test_ship_router)

# Visual Studio에서 실행 시 작업 디렉토리를 LINK 폴더로 설정
set_target_properties(test_ship_router PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/../../.."
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
message(STATUS "  [PYTHON] algorithm_module: bindings.cpp") 
message(STATUS "")
message(STATUS "Targets:")
message(STATUS "  algorithm_module      - Python binding (.pyd)")
message(STATUS "  test_grid_snapper     - Grid & Snapping test (optional)")
message(STATUS "  test_ship_router      - Full integration test (optional)")
message(STATUS "")
message(STATUS "Auto-copy on build:")
message(STATUS "  - algorithm_module.pyd -> LINK/")
message(STATUS "  - ShipDynamics.dll -> LINK/")
message(STATUS "")
message(STATUS "Data files required in LINK/data/:")
message(STATUS "  - gebco/GEBCO_2024_sub_ice_topo.nc")
message(STATUS "  - gshhs/GSHHS_i_L1.shp")
message(STATUS "  - weather/*.bin (optional)")
message(STATUS "  - dll/ShipDynamics.dll")
message(STATUS "================================")