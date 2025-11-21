#include "../../include/fetch_data.h"
#include <iostream>
#include <gdal_priv.h>

FetchData::FetchData()
    : bathymetryLoaded(false), coastlineLoaded(false) {
    GDALAllRegister();
}

FetchData::~FetchData() {
    gebcoLoader.reset();
    gshhsLoader.reset();
    GDALDestroyDriverManager();
}

bool FetchData::LoadBathymetryData(const std::string& path) {
    gebcoLoader = std::make_unique<GebcoLoader>(path);
    bathymetryLoaded = gebcoLoader->Open();
    return bathymetryLoaded;
}

bool FetchData::LoadCoastlineData(const std::string& path) {
    gshhsLoader = std::make_unique<GshhsLoader>(path);
    coastlineLoaded = gshhsLoader->Open();
    return coastlineLoaded;
}

BoundingBox FetchData::CalculateBaseROI(const std::vector<GeoCoordinate>& waypoints) {
    return BoundingBox::FromWaypoints(waypoints);
}

NavigableGrid FetchData::BuildNavigableGrid(
    const std::vector<GeoCoordinate>& waypoints,
    double targetCellSizeKm,
    int marginCells
    )
{
    if (!bathymetryLoaded || !coastlineLoaded) {
        throw std::runtime_error("Data not loaded: Bathymetry or Coastline data is missing.");
    }

	// Step 1: Calculate base ROI from waypoints (no margin)
	BoundingBox baseROI = CalculateBaseROI(waypoints);

#ifdef _DEBUG
    std::cout << "1) Data Extraction [pixel]\n";
    std::cout << "[BaseROI] lat: [" << baseROI.minLat << ", " << baseROI.maxLat
        << "], lon: [" << baseROI.minLon << ", " << baseROI.maxLon << "]\n";
#endif

	// Step 2: Calculate block size first to determine pixel margin
    auto [blockLat, blockLon] = GridResolution::CalculateBlockSize(
        baseROI,
        targetCellSizeKm
	);

    // Step 3: Calculate dynamic pixel margin based on marginCells and block size
	int pixelMargin = std::max(blockLat, blockLon) * marginCells;

#ifdef _DEBUG
    std::cout << "  - Margin calculation: " << marginCells << " cells ¡¿ "
        << std::max(blockLat, blockLon) << " pixels/cell = "
        << pixelMargin << " pixels\n";
#endif

	// Step 4: Extract GEBCO data with pixel margin (create Expanded ROI)
    std::vector<std::vector<float>> depths;
	double latStepGeo = 0.0, lonStepGeo = 0.0;
    BoundingBox expandedROI;
    if (!gebcoLoader->ExtractROI(
		baseROI,
        depths,
        latStepGeo,
        lonStepGeo,
        pixelMargin,
        expandedROI
    )) {
		throw std::runtime_error("Failed to extract GEBCO data");
    }
#ifdef _DEBUG
    std::cout << "  - Extract Source [GEBCO]: " << depths.size() << "x"
        << (depths.empty() ? 0 : depths[0].size()) << " (Depths)\n";
#endif

	// Step 5: Extract GSHHS polygons using the same Expanded ROI
    const auto polygons = gshhsLoader->ExtractROI(expandedROI);
#ifdef _DEBUG
    std::cout << "  - Extract Source [GSHHS]: " << polygons.size() << " (Polygons)\n";
#endif

	// Step 6: Caclulate full grid resoluton with actual data dimensions
    auto gridResolutoin = GridResolution::Calculate(
        expandedROI,
        static_cast<int>(depths.size()),
        static_cast<int>(depths.empty() ? 0 : depths[0].size()),
        targetCellSizeKm
    );

	// Step 7: Create and build NavigableGrid
    NavigableGrid grid(expandedROI, gridResolutoin.rows, gridResolutoin.cols);
    grid.BuildFromData(expandedROI, depths, polygons, gridResolutoin.blockLat, gridResolutoin.blockLon);

    return grid;
}



