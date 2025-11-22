#include "grid_builder.h"
#include <iostream>
#include <gdal_priv.h>
#include <gdal_alg.h>

GridBuilder::GridBuilder()
    : bathymetryLoaded_(false), coastlineLoaded_(false) {
    GDALAllRegister();
}

GridBuilder::~GridBuilder() {
    gebcoLoader_.reset();
    gshhsLoader_.reset();
    GDALDestroyDriverManager();
}

bool GridBuilder::LoadBathymetryData(const std::string& path) {
    gebcoLoader_ = std::make_unique<GebcoLoader>(path);
    bathymetryLoaded_ = gebcoLoader_->Open();
    return bathymetryLoaded_;
}

bool GridBuilder::LoadCoastlineData(const std::string& path) {
    gshhsLoader_ = std::make_unique<GshhsLoader>(path);
    coastlineLoaded_ = gshhsLoader_->Open();
    return coastlineLoaded_;
}

BoundingBox GridBuilder::CalculateBaseROI(const std::vector<GeoCoordinate>& waypoints) {
    return BoundingBox::FromWaypoints(waypoints);
}

NavigableGrid GridBuilder::BuildNavigableGrid(
    const std::vector<GeoCoordinate>& waypoints,
    double targetCellSizeKm,
    int marginCells)
{
    if (!bathymetryLoaded_ || !coastlineLoaded_) {
        throw std::runtime_error("Data not loaded");
    }

    // Step 1: Calculate base ROI
    BoundingBox baseROI = CalculateBaseROI(waypoints);

// #ifdef _DEBUG
//     std::cout << "1) Data Extraction [pixel]\n";
//     std::cout << "[BaseROI] lat: [" << baseROI.minLat << ", " << baseROI.maxLat
//         << "], lon: [" << baseROI.minLon << ", " << baseROI.maxLon << "]\n";
// #endif

    // Step 2: Calculate block size
    auto [blockLat, blockLon] = GridResolution::CalculateBlockSize(
        baseROI, targetCellSizeKm
    );

    // Step 3: Calculate pixel margin
    int pixelMargin = std::max(blockLat, blockLon) * marginCells;

// #ifdef _DEBUG
//     std::cout << "  - Margin: " << marginCells << " cells x "
//         << std::max(blockLat, blockLon) << " pixels/cell = "
//         << pixelMargin << " pixels\n";
// #endif

    // Step 4: Extract GEBCO data
    std::vector<std::vector<float>> depths;
    double latStepGeo, lonStepGeo;
    BoundingBox expandedROI;
    
    if (!gebcoLoader_->ExtractROI(baseROI, depths, latStepGeo, lonStepGeo, 
                                   pixelMargin, expandedROI)) {
        throw std::runtime_error("Failed to extract GEBCO data");
    }

// #ifdef _DEBUG
//     std::cout << "  - GEBCO: " << depths.size() << "x"
//         << (depths.empty() ? 0 : depths[0].size()) << "\n";
// #endif

    // Step 5: Extract GSHHS polygons
    auto polygons = gshhsLoader_->ExtractROI(expandedROI);

// #ifdef _DEBUG
//     std::cout << "  - GSHHS: " << polygons.size() << " polygons\n";
// #endif

    // Step 6: Calculate grid resolution
    auto gridRes = GridResolution::Calculate(
        expandedROI,
        static_cast<int>(depths.size()),
        static_cast<int>(depths.empty() ? 0 : depths[0].size()),
        targetCellSizeKm
    );

    // Step 7: Create grid
    NavigableGrid grid(expandedROI, gridRes.rows, gridRes.cols);

    // Step 8: Downsample depths
    auto downsampled = DownsampleDepths(depths, gridRes.rows, gridRes.cols, 
                                        gridRes.blockLat, gridRes.blockLon);

    // Step 9: Rasterize GSHHS
    auto landMask = RasterizeGSHHS_GDAL(polygons, grid);

    // Step 10: Apply masks
    BuildMask(grid, downsampled, landMask);

    return grid;
}

// ========== 여기서부터 기존 navigable_grid.cpp 코드를 복사 ==========

std::vector<std::vector<float>> GridBuilder::DownsampleDepths(
    const std::vector<std::vector<float>>& originalDepths,
    int targetRows,
    int targetCols,
    int blockLat,
    int blockLon)
{
    // --- Basic validation ---
    if (originalDepths.empty() || originalDepths[0].empty())
        throw std::runtime_error("DownsampleDepths: empty input depths");
    if (blockLat <= 0 || blockLon <= 0)
        throw std::invalid_argument("DownsampleDepths: blockLat/blockLon must be positive");

    const int srcRows = static_cast<int>(originalDepths.size());
    const int srcCols = static_cast<int>(originalDepths[0].size());

    // --- Calculate recommended targetRows/Cols based on block (always downsample) ---
    const int expectedRows = std::max(1, srcRows / blockLat);
    const int expectedCols = std::max(1, srcCols / blockLon);

    // If requested targetRows/Cols are different, adjust (clamp) to block-based values
    if (targetRows != expectedRows || targetCols != expectedCols) {
        std::cout << "[Downsample] Adjust grid size to block-aligned:"
            << " requested " << targetRows << "x" << targetCols
            << " -> " << expectedRows << "x" << expectedCols << "\n";
        targetRows = expectedRows;
        targetCols = expectedCols;
    }

    // Prevent upsampling: if block is larger than source but target is larger, throw exception
    if (targetRows > srcRows || targetCols > srcCols) {
        throw std::runtime_error("DownsampleDepths: upsampling detected; grid exceeds source pixels");
    }

    std::vector<std::vector<float>> downsampled(
        targetRows, std::vector<float>(targetCols, 0.0f)
    );

// #ifdef _DEBUG
//     std::cout << "[Downsampling] " << srcRows << "x" << srcCols
//         << " -> " << targetRows << "x" << targetCols << "\n";
// #endif

    for (int dstRow = 0; dstRow < targetRows; ++dstRow) {
        // Source range covered by this output row (row start/end, end is exclusive)
        const int srcRowStart = dstRow * blockLat;
        const int srcRowEnd = std::min(srcRowStart + blockLat, srcRows);

        for (int dstCol = 0; dstCol < targetCols; ++dstCol) {
            const int srcColStart = dstCol * blockLon;
            const int srcColEnd = std::min(srcColStart + blockLon, srcCols);

            double sum = 0.0;
            int count = 0;

            // Average of blockLat × blockLon (including edge clipping)
            for (int r = srcRowStart; r < srcRowEnd; ++r) {
                const auto& row = originalDepths[r];
                for (int c = srcColStart; c < srcColEnd; ++c) {
                    sum += row[c];
                    ++count;
                }
            }

            downsampled[dstRow][dstCol] = (count > 0) ? static_cast<float>(sum / count) : 0.0f;
        }
    }
    return downsampled;
}

std::vector<std::vector<bool>> GridBuilder::RasterizeGSHHS_GDAL(
    const std::vector<GSHHSPolygon>& polygons,
    const NavigableGrid& grid)
{
    int rows = grid.Rows();
    int cols = grid.Cols();
    const BoundingBox& geoBounds = grid.Bounds();

// #ifdef _DEBUG
//     std::cout << "[Rasterize] Rasterizing "
//         << polygons.size() << " GSHHS polygons...\n";
// #endif

    // 1) Prepare memory vector driver ("MEM")
    GDALDriver* memVectorDriver = GetGDALDriverManager()->GetDriverByName("MEM");
    if (!memVectorDriver) {
        std::cerr << "[ERROR] OGR Memory driver not available\n";
        return {};
    }
    // 1-1) Create vector dataset (container for polygons)
    GDALDataset* vectorDS = memVectorDriver->Create("", 0, 0, 0, GDT_Unknown, nullptr);
    if (!vectorDS) {
        std::cerr << "[ERROR] Failed to create vector dataset\n";
        return {};
    }

    // 2) Prepare raster driver ("MEM")
    GDALDriver* memRasterDriver = GetGDALDriverManager()->GetDriverByName("MEM");
    if (!memRasterDriver) {
        std::cerr << "[ERROR] MEM raster driver not available\n";
        GDALClose(vectorDS);
        return {};
    }
    // 2-1) Create raster dataset (cols x rows, single band Byte)
    GDALDataset* rasterDS = memRasterDriver->Create("", cols, rows, 1, GDT_Byte, nullptr);
    if (!rasterDS) {
        std::cerr << "[ERROR] Failed to create raster dataset\n";
        GDALClose(vectorDS);
        return {};
    }

    // 3) Set coordinate system (EPSG:4326 = WGS84 lat/lon)
    OGRSpatialReference srs;
    srs.SetFromUserInput("EPSG:4326"); // new way
    //srs.SetWellKnownGeogCS("WGS84"); // old way

    // 3-1) Create vector layer (layer to hold polygons)
    OGRLayer* layer = vectorDS->CreateLayer("polygons", &srs, wkbPolygon, nullptr);
    if (!layer) {
        std::cerr << "[ERROR] Failed to create layer\n";
        GDALClose(vectorDS);
        return {};
    }

    // 4) Convert GSHHS polygons to OGR Features and insert into layer
    int landCount = 0;
    for (const auto& gshhsPoly : polygons) {
        if (gshhsPoly.level != 1) continue;  // level 1 = land
        if (gshhsPoly.points.size() < 3) continue;  // Less than a triangle is invalid

        OGRFeature* feature = OGRFeature::CreateFeature(layer->GetLayerDefn());
        OGRPolygon ogrPoly;
        OGRLinearRing ring;

        for (const auto& pt : gshhsPoly.points) {
            ring.addPoint(pt.longitude, pt.latitude);
        }
        ring.closeRings();
        ogrPoly.addRing(&ring);
        feature->SetGeometry(&ogrPoly);

        if (layer->CreateFeature(feature) != OGRERR_NONE) {
            std::cerr << "[WARNING] Failed to create feature\n";
        }
        else {
            landCount++;
        }

        OGRFeature::DestroyFeature(feature);
    }

    // 5) Set raster GeoTransform/projection
    // Specify top-left origin and pixel size (latitude decreases, so negative)
    double geoTransform[6] = {
        geoBounds.minLon,           // Top-left X
        geoBounds.Width() / cols,   // Pixel width
        0.0,
        geoBounds.maxLat,           // Top-left Y
        0.0,
        -geoBounds.Height() / rows  // Pixel height (negative)
    };
    rasterDS->SetGeoTransform(geoTransform);

    char* wkt = nullptr;
    srs.exportToWkt(&wkt);
    rasterDS->SetProjection(wkt);
    CPLFree(wkt);

    // 6) Initialize band (0: sea)
    GDALRasterBand* band = rasterDS->GetRasterBand(1);
#if GDAL_VERSION_NUM >= 2010000
    band->Fill(0.0);
#else
    {
        std::vector<uint8_t> zeroBuffer(static_cast<size_t>(rows) * cols, 0);
        band->RasterIO(GF_Write, 0, 0, cols, rows, zeroBuffer.data(),
            cols, rows, GDT_Byte, 0, 0);
    }
#endif

    // 7) Prepare rasterization options
    int bandList[] = { 1 };
    double burnValue[] = { 1.0 }; // land = 1
    char** options = nullptr;
    options = CSLSetNameValue(options, "ALL_TOUCHED", "TRUE"); // Fill boundary cells

    // 8) Burn layer -> raster with burn value (1)
    OGRLayerH hLayer = (OGRLayerH)layer;
    OGRLayerH layers[] = { hLayer };
    CPLErr err = GDALRasterizeLayers(
        rasterDS,
        1, bandList, 
        1, layers, 
        nullptr, nullptr, 
        burnValue, 
        options, 
        nullptr, nullptr
    );
    CSLDestroy(options);

    if (err != CE_None) {
        std::cerr << "[ERROR] GDALRasterizeLayers failed!\n";
        GDALClose(rasterDS);
        GDALClose(vectorDS);
        return {};
    }

    // 9) Read the resulting raster into a bool mask
    std::vector<uint8_t> buffer(static_cast<size_t>(rows) * cols);
    if (band->RasterIO(GF_Read, 0, 0, cols, rows, buffer.data(),
        cols, rows, GDT_Byte, 0, 0) != CE_None) {
        std::cerr << "[ERROR] Failed to read raster band\n";
        GDALClose(rasterDS);
        GDALClose(vectorDS);
        return {};
    }

    std::vector<std::vector<bool>> landMask(rows, std::vector<bool>(cols, false));
    for (int r = 0; r < rows; ++r) {
        const size_t base = static_cast<size_t>(r) * cols;
        for (int c = 0; c < cols; ++c) {
            landMask[r][c] = (buffer[base + c] == 1);
        }
    }
    // 10) Clean up resources
    GDALClose(rasterDS);
    GDALClose(vectorDS);
    return landMask;
}

void GridBuilder::BuildMask(
    NavigableGrid& grid,
    const std::vector<std::vector<float>>& downsampledDepths,
    const std::vector<std::vector<bool>>& landMask)
{
// #ifdef _DEBUG
//     std::cout << "\n3) Applying Grid Masks (GEBCO & GSHHS)\n";
// #endif

    // Step 1: GEBCO-based classification
    for (int r = 0; r < grid.Rows(); ++r) {
        for (int c = 0; c < grid.Cols(); ++c) {
            float depth = downsampledDepths[r][c];

            if (depth >= 0.0f) {
                grid.SetCellType(r, c, CellType::LAND);
            }
            else if (depth > -15.0f) {
                grid.SetCellType(r, c, CellType::SHALLOW);
            }
            else {
                grid.SetCellType(r, c, CellType::NAVIGABLE);
            }
        }
    }

// #ifdef _DEBUG
//     std::cout << "[Build] Applying GEBCO depth classification...\n";
// #endif

    // Step 2: GSHHS overwrite
    int gshhsOverwrites = 0;
    for (int r = 0; r < grid.Rows(); ++r) {
        for (int c = 0; c < grid.Cols(); ++c) {
            if (landMask[r][c]) {
                if (grid.GetCellType(r, c) != CellType::LAND) {
                    gshhsOverwrites++;
                }
                grid.SetCellType(r, c, CellType::LAND);
            }
        }
    }

// #ifdef _DEBUG
//     std::cout << "[Build] Applying GSHHS land mask ("
//         << gshhsOverwrites << " cells corrected)...\n";
// #endif

    PrintStatistics(grid);
}

void GridBuilder::PrintStatistics(const NavigableGrid& grid) const
{
    int rows = grid.Rows();
    int cols = grid.Cols();
    
    int landCount = 0, shallowCount = 0, navigableCount = 0;
    
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            switch (grid.GetCellType(r, c)) {
            case CellType::LAND: landCount++; break;
            case CellType::SHALLOW: shallowCount++; break;
            case CellType::NAVIGABLE: navigableCount++; break;
            default: break;
            }
        }
    }
    
    int total = rows * cols;
    
#ifdef _DEBUG
    std::cout << "\n[Grid Statistics] "
        << "Total cells: " << total << "\n"
        << " > LAND: " << landCount << " ("
        << landCount * 100.0 / total << "%)\n"
        << " > SHALLOW: " << shallowCount << " ("
        << shallowCount * 100.0 / total << "%)\n"
        << " > NAVIGABLE: " << navigableCount << " ("
        << navigableCount * 100.0 / total << "%)\n";
#endif
}