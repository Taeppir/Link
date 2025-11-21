#include "data_loading/grid_builder.h"
#include "types/geo_types.h"
#include <iostream>

int main() {
    try {
        std::cout << "=== Grid Builder Test ===" << std::endl;

        // GridBuilder 생성
        GridBuilder builder;

        // GEBCO 데이터 로드
        std::string gebcoPath = "../data/GEBCO_2024_sub_ice_topo.nc";
        if (builder.LoadBathymetryData(gebcoPath)) {
            std::cout << "✓ GEBCO loaded successfully" << std::endl;
        } else {
            std::cerr << "✗ Failed to load GEBCO" << std::endl;
            return 1;
        }

        // GSHHS 데이터 로드
        std::string gshhsPath = "../data/GSHHS_i_L1.shp";
        if (builder.LoadCoastlineData(gshhsPath)) {
            std::cout << "✓ GSHHS loaded successfully" << std::endl;
        } else {
            std::cerr << "✗ Failed to load GSHHS" << std::endl;
            return 1;
        }

        // 테스트 웨이포인트 (부산 -> 제주)
        std::vector<GeoCoordinate> waypoints = {
            {35.0994, 129.0336},  // 부산
            {33.4996, 126.5312}   // 제주
        };

        std::cout << "\n=== Building Grid ===" << std::endl;

        // 그리드 생성 (Debug 모드면 자동으로 로그 출력)
        NavigableGrid grid = builder.BuildNavigableGrid(waypoints, 5.0, 5);

        std::cout << "\n=== Grid Built Successfully ===" << std::endl;
        std::cout << "Grid size: " << grid.Rows() << " x " << grid.Cols() << std::endl;

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}