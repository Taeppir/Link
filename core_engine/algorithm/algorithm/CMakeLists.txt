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
# GDAL 설정
# ============================================
set(GDAL_INCLUDE_DIR "C:/vcpkg/installed/x64-windows/include")
set(GDAL_LIBRARY "C:/vcpkg/installed/x64-windows/lib/gdal.lib")
find_package(GDAL QUIET)

message(STATUS "GDAL Include: ${GDAL_INCLUDE_DIR}")
message(STATUS "GDAL Library: ${GDAL_LIBRARY}")

# ============================================
# 1. types 라이브러리 (헤더 대부분, grid_types.cpp만 있음)
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
# 빌드 정보 출력
# ============================================
message(STATUS "================================")
message(STATUS "Build Configuration:")
message(STATUS "  types: grid_types.cpp")
message(STATUS "  utils: 4 files")
message(STATUS "  data_loading: 4 files")
message(STATUS "================================")