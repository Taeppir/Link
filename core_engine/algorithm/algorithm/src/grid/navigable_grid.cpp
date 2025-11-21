#include "../../include/navigable_grid.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <gdal_alg.h>

std::pair<int, int> GridResolution::CalculateBlockSize(
    const BoundingBox& roi,
    double targetCellSizeKm)
{
    using namespace GEBCOConstants;

    double avgLat = (roi.minLat + roi.maxLat) / 2.0;
    const double kmPerPixLon = KM_PER_PIXEL_LAT * std::cos(avgLat * M_PI / 180.0);

    int blockLat = std::max(1, static_cast<int>(std::round(targetCellSizeKm / KM_PER_PIXEL_LAT)));
    int blockLon = std::max(1, static_cast<int>(std::round(targetCellSizeKm / kmPerPixLon)));

    return { blockLat, blockLon };
}



GridResolution GridResolution::Calculate(
    const BoundingBox& roi,
    int srcHeight, int srcWidth,
    double targetCellSizeKm)
{
    GridResolution res{};
    res.cellSizeKm = targetCellSizeKm;

	auto [blockLat, blockLon] = CalculateBlockSize(roi, targetCellSizeKm);
	res.blockLat = blockLat;
	res.blockLon = blockLon;

    res.rows = std::max(1, srcHeight / blockLat);
    res.cols = std::max(1, srcWidth / blockLon);

    const int MAX_GRID_SIZE = 9000;
    res.rows = std::min(res.rows, MAX_GRID_SIZE);
    res.cols = std::min(res.cols, MAX_GRID_SIZE);

#ifdef _DEBUG
    std::cout << "\n2) Grid Definition [cell]\n";
    std::cout << "[TargetCellSize] " << targetCellSizeKm << "km\n";
    std::cout << "[GridResolution]\n";
    std::cout << "  - Block Size (lat/lon): " << res.blockLat << "x" << res.blockLon << " [pixels]\n";
#endif
    return res;
}


NavigableGrid::NavigableGrid()
	: rows(0), cols(0), cellSizeLat(0), cellSizeLon(0), mapper_() {
}
NavigableGrid::NavigableGrid(const BoundingBox& roi, int rows, int cols)
	: geoBounds(roi), rows(rows), cols(cols), mapper_(roi, rows, cols)
{
	grid.resize(rows, std::vector<CellType>(cols, CellType::UNKNOWN));

	cellSizeLat = roi.Height() / rows;
	cellSizeLon = roi.Width() / cols;

#ifdef _DEBUG
    std::cout << "  - Cell Size (approx): " << cellSizeLat << "° x " << cellSizeLon << "°\n";
#endif

    	
}

GeoCoordinate NavigableGrid::GridToGeo(int row, int col) const {
	return mapper_.GridToGeo(GridCoordinate(row, col));
}
GeoCoordinate NavigableGrid::GridToGeo(const GridCoordinate& grid) const {
	return mapper_.GridToGeo(grid);
}

GridCoordinate NavigableGrid::GeoToGrid(const GeoCoordinate& geo) const {
	return mapper_.GeoToGrid(geo);
}

CellType NavigableGrid::GetCellType(int row, int col) const {
	if (!IsValid(row, col)) return CellType::UNKNOWN;
	return grid[row][col];
}

bool NavigableGrid::IsNavigable(int row, int col) const {
	return IsValid(row, col) && grid[row][col] == CellType::NAVIGABLE;
}

bool NavigableGrid::IsValid(int row, int col) const {
	return row >= 0 && row < rows && col >= 0 && col < cols;
}

bool NavigableGrid::IsValid(const GridCoordinate& pos) const {
	return IsValid(pos.row, pos.col);
}

// GEBCO Downampling
std::vector<std::vector<float>> NavigableGrid::DownsampleDepths(
    const std::vector<std::vector<float>>& originalDepths,
    int targetRows,
    int targetCols,
    int blockLat,     // Number of GEBCO 15" pixels to group in latitude direction
    int blockLon      // Number of GEBCO 15" pixels to group in longitude direction
)
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

#ifdef _DEBUG
    std::cout << "[Downsampling] " << srcRows << "x" << srcCols
        << " -> " << targetRows << "x" << targetCols << "\n";
#endif

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
                // (옵션) 행마다 길이가 같은지 보장되지 않는 입력에 대비하려면 다음 가드 사용:
                // if ((int)originalDepths[r].size() != srcCols) { ... }
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


// GSHHS Rasterization
std::vector<std::vector<bool>> NavigableGrid::RasterizeGSHHS_GDAL(
    const std::vector<GSHHSPolygon>& polygons)
{
#ifdef _DEBUG
    std::cout << "[Rasterize] Rasterizing "
        << polygons.size() << " GSHHS polygons...\n";
#endif

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

// Masking
void NavigableGrid::BuildMask(
    const std::vector<std::vector<float>>& downsampledDepths,
    const std::vector<std::vector<bool>>& landMask)
{
#ifdef _DEBUG
    std::cout << "\n3) Applying Grid Masks (GEBCO & GSHHS)\n";
#endif

    // Step 1: GEBCO-based classification
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            float depth = downsampledDepths[r][c];

            if (depth >= 0.0f) {
                grid[r][c] = CellType::LAND;
            }
            else if (depth > -15.0f) {
                grid[r][c] = CellType::SHALLOW;
            }
            else {
                grid[r][c] = CellType::NAVIGABLE;
            }
        }
    }

#ifdef _DEBUG
    std::cout << "[Build] Applying GEBCO depth classification...\n";
#endif

    // Step 2: GSHHS overwrite
    int gshhsOverwrites = 0;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (landMask[r][c]) {
                if (grid[r][c] != CellType::LAND) {
                    gshhsOverwrites++;
                }
                grid[r][c] = CellType::LAND;
            }
        }
    }

#ifdef _DEBUG
    std::cout << "[Build] Applying GSHHS land mask ("
        << gshhsOverwrites << " cells corrected)...\n";
#endif

    PrintStatistics();
}


void NavigableGrid::BuildFromData(
    const BoundingBox& depthBounds,
    const std::vector<std::vector<float>>& gebcoDepths, 
    const std::vector<GSHHSPolygon>& gshhsPolygons,
    int blockLat, int blockLon
    )
{

    // Step 1: GEBCO Downampling
    auto downsampled = DownsampleDepths(gebcoDepths, rows, cols, blockLat, blockLon);

    // Step 2: GSHHS Rasterization
    auto landMask = RasterizeGSHHS_GDAL(gshhsPolygons);

    if (landMask.empty()) {
        std::cerr << "[ERROR] Failed to rasterize GSHHS data!\n";
        return;
    }

    // Step 3: Masking
    BuildMask(downsampled, landMask);
}

void NavigableGrid::SetAllCells(CellType cellType) {
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            grid[r][c] = cellType;
        }
    }
}

void NavigableGrid::PrintStatistics() const {
    int landCount = 0, shallowCount = 0, navigableCount = 0;

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            switch (grid[r][c]) {
            case CellType::LAND: landCount++; break;
            case CellType::SHALLOW: shallowCount++; break;
            case CellType::NAVIGABLE: navigableCount++; break;
            default: break;
            }
        }
    }

    int total = rows * cols;
#ifdef _DEBUG
    std::cout << "[Grid Statistics] "
        << "Total cells: " << total << "\n"
        << " > LAND: " << landCount << " ("
        << landCount * 100.0 / total << "%)\n"
        << " > SHALLOW: " << shallowCount << " ("
        << shallowCount * 100.0 / total << "%)\n"
        << " > NAVIGABLE: " << navigableCount << " ("
        << navigableCount * 100.0 / total << "%)\n";
#endif
}