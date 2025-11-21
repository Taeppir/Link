#pragma once
#include "gebco_loader.h"
#include "gshhs_loader.h"
#include "common_types.h"
#include "navigable_grid.h"
#include <memory>
#include <string>

class FetchData {
private:
    std::unique_ptr<GebcoLoader> gebcoLoader;
    std::unique_ptr<GshhsLoader> gshhsLoader;

    bool bathymetryLoaded;
    bool coastlineLoaded;

public:
    FetchData();
    ~FetchData();

    // Load data files
    bool LoadBathymetryData(const std::string& path);
    bool LoadCoastlineData(const std::string& path);

    bool IsBathymetryLoaded() const { return bathymetryLoaded; }
    bool IsCoastlineLoaded() const { return coastlineLoaded; }
	
    // calculate ROI from waypoints without margin
    BoundingBox CalculateBaseROI(const std::vector<GeoCoordinate>& waypoints);

    NavigableGrid BuildNavigableGrid(
        const std::vector<GeoCoordinate>& waypoints,
        double targetCellSizeKm = 1.0,   // default 1km grid cells,
		int marginCells = 3		         // default 3 cells margin
    );
};