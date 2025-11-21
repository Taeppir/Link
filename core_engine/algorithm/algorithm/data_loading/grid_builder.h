#pragma once
#include "gebco_loader.h"
#include "gshhs_loader.h"
#include "../types/geo_types.h"
#include "../types/grid_types.h"
#include <memory>
#include <string>

// ===== Grid Builder (Complex Build Logic) =====
class GridBuilder {
public:
    GridBuilder();
    ~GridBuilder();

    // Data loading
    bool LoadBathymetryData(const std::string& path);
    bool LoadCoastlineData(const std::string& path);

    bool IsBathymetryLoaded() const { return bathymetryLoaded_; }
    bool IsCoastlineLoaded() const { return coastlineLoaded_; }
    
    // Main build function
    NavigableGrid BuildNavigableGrid(
        const std::vector<GeoCoordinate>& waypoints,
        double targetCellSizeKm = 1.0,
        int marginCells = 3
    );

private:
    std::unique_ptr<GebcoLoader> gebcoLoader_;
    std::unique_ptr<GshhsLoader> gshhsLoader_;

    bool bathymetryLoaded_;
    bool coastlineLoaded_;

    // Helper functions (기존 navigable_grid.cpp의 로직들)
    BoundingBox CalculateBaseROI(const std::vector<GeoCoordinate>& waypoints);

    std::vector<std::vector<float>> DownsampleDepths(
        const std::vector<std::vector<float>>& originalDepths,
        int targetRows, int targetCols,
        int blockLat, int blockLon
    );

    std::vector<std::vector<bool>> RasterizeGSHHS_GDAL(
        const std::vector<GSHHSPolygon>& polygons,
        const NavigableGrid& grid
    );

    void BuildMask(
        NavigableGrid& grid,
        const std::vector<std::vector<float>>& downsampledDepths,
        const std::vector<std::vector<bool>>& landMask
    );

    void PrintStatistics(const NavigableGrid& grid) const;
};