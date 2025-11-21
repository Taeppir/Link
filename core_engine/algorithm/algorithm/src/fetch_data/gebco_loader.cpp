#include "../../include/gebco_loader.h"
#include <iostream>
#include <cmath>

GebcoLoader::GebcoLoader(const std::string& filepath)
    : filepath(filepath), dataset(nullptr), band(nullptr),
    rasterWidth(0), rasterHeight(0) {
    for (int i = 0; i < 6; ++i) {
        geoTransform[i] = 0.0;
		invGeoTransform[i] = 0.0;
    }
}

GebcoLoader::~GebcoLoader() {
    Close();
}

bool GebcoLoader::Open() {
    dataset = (GDALDataset*)GDALOpen(filepath.c_str(), GA_ReadOnly);
    if (!dataset) {
        std::cerr << "[ERROR] Failed to open GEBCO file: " << filepath << std::endl;
        return false;
    }

    rasterWidth = dataset->GetRasterXSize();
    rasterHeight = dataset->GetRasterYSize();

    if (dataset->GetGeoTransform(geoTransform) != CE_None) {
        std::cerr << "[ERROR] Failed to get geotransform" << std::endl;
        Close();
        return false;
    }

    if (!InitInverseGeoTransform()) {
        Close();
        return false;
    }

#ifdef _DEBUG
    std::cout << "[DEBUG][GebcoLoader] Opened: " << filepath << "\n"
        << "  - Raster Size : " << rasterWidth << " x " << rasterHeight << "\n"
        << "  - GeoTransform: [originLon=" << geoTransform[0]
        << ", originLat=" << geoTransform[3] << "] "
        << "Res: [dLon=" << geoTransform[1]
        << ", dLat=" << geoTransform[5] << "]"
        << std::endl;
#endif

    band = dataset->GetRasterBand(1);
    if (!band) {
        std::cerr << "[ERROR] Failed to get raster band" << std::endl;
        Close();
        return false;
    }
    return true;
}

void GebcoLoader::Close() {
    if (dataset) {
        GDALClose(dataset);
        dataset = nullptr;
        band = nullptr;
    }
}

bool GebcoLoader::InitInverseGeoTransform() {
    if (GDALInvGeoTransform(geoTransform, invGeoTransform) == FALSE) {
        std::cerr << "[ERROR] Failed to compute inverse geotransfom." << std::endl;
        return false;
    }
    return true;
}

void GebcoLoader::GeoToPixelCont(double lon, double lat, double& pixelX, double& pixelY) const {
    pixelX = invGeoTransform[0] + lon * invGeoTransform[1] + lat * invGeoTransform[2];
    pixelY = invGeoTransform[3] + lon * invGeoTransform[4] + lat * invGeoTransform[5];
}

void GebcoLoader::PixelCornerToGeo(int col, int row, double& lon, double& lat) const {
    lon = geoTransform[0] + static_cast<double>(col) * geoTransform[1] + static_cast<double>(row) * geoTransform[2];
    lat = geoTransform[3] + static_cast<double>(col) * geoTransform[4] + static_cast<double>(row) * geoTransform[5];
}


void  GebcoLoader::GeoToPixelIndexTopLeft(double lon, double lat, int& col, int& row) const {
    double pixelX, pixelY;
    GeoToPixelCont(lon, lat, pixelX, pixelY);
    col = static_cast<int>(std::floor(pixelX));
    row = static_cast<int>(std::floor(pixelY));

}
void  GebcoLoader::GeoToPixelIndexBottomRight(double lon, double lat, int& col, int& row) const {
    double pixelX, pixelY;
    GeoToPixelCont(lon, lat, pixelX, pixelY);
    col = static_cast<int>(std::ceil(pixelX)) - 1;
    row = static_cast<int>(std::ceil(pixelY)) - 1;
}

bool GebcoLoader::ExpandROIWithPixelMargin(const BoundingBox& baseROI,int marginPixel,PixelWindow& pixelWindow, BoundingBox& expandedROI
) const {
    if (!IsOpen()) return false;

    // Step 1: Snap
    GeoToPixelIndexTopLeft(baseROI.minLon, baseROI.maxLat, pixelWindow.leftCol, pixelWindow.topRow);
    GeoToPixelIndexBottomRight(baseROI.maxLon, baseROI.minLat, pixelWindow.rightCol, pixelWindow.bottomRow);

    // Step 2: add pixel margin
    pixelWindow.leftCol -= marginPixel;
    pixelWindow.topRow -= marginPixel;
    pixelWindow.rightCol += marginPixel;
    pixelWindow.bottomRow += marginPixel;

    // Step 3: clamp bounds
    pixelWindow.leftCol = std::clamp(pixelWindow.leftCol, 0, rasterWidth - 1);
    pixelWindow.rightCol = std::clamp(pixelWindow.rightCol, 0, rasterWidth - 1);
    pixelWindow.topRow = std::clamp(pixelWindow.topRow, 0, rasterHeight - 1);
    pixelWindow.bottomRow = std::clamp(pixelWindow.bottomRow, 0, rasterHeight - 1);

    if (pixelWindow.width() <= 0 || pixelWindow.height() <= 0) return false;

    // Step 4: caculate expandedROI
    double westLon, northLat, eastLon, southLat;
    PixelCornerToGeo(pixelWindow.leftCol, pixelWindow.topRow, westLon, northLat);
    PixelCornerToGeo(pixelWindow.rightCol + 1, pixelWindow.bottomRow + 1, eastLon, southLat);

    expandedROI = BoundingBox(
        southLat, // minLat
        northLat, // maxLat
        westLon,  // minLon
        eastLon   // maxLon
    );

    return true;
}


bool GebcoLoader::ExtractROI(
    const BoundingBox& baseROI,
    std::vector<std::vector<float>>& depths,
    double& latStepGeo,
    double& lonStepGeo,
	int pixelMargin,
    BoundingBox& expandedROI
    ) const {
    if (!IsOpen()) {
        std::cerr << "[ERROR] GEBCO dataset not opened" << std::endl;
        return false;
    }

    // Step 1: Calculate pixel window with margin expansion
    PixelWindow pixelWindow;
    if (!ExpandROIWithPixelMargin(baseROI, pixelMargin, pixelWindow, expandedROI)) {
        std::cerr << "[ERROR] Invalid ROI window\n";
        return false;
    }

#ifdef _DEBUG
    /*
    std::cout << "[ExtractROI] GEBCO window: Left=" << pixelWindow.leftCol
        << " Top=" << pixelWindow.topRow
        << " Rigt=" << pixelWindow.rightCol
        << " Bottom=" << pixelWindow.bottomRow
        << " (width=" << pixelWindow.width()
        << ", height=" << pixelWindow.height() << ")\n";
    */
    std::cout << "[ExpandedROI] lat: [" << expandedROI.minLat
        << ", " << expandedROI.maxLat
        << "], lon: [" << expandedROI.minLon
        << ", " << expandedROI.maxLon << "]\n";
#endif 
    
	// Step 2: Read Data with RasterIO()
    int width = pixelWindow.width();
    int height = pixelWindow.height();
    std::vector<float> buffer(width * height);
    CPLErr err = band->RasterIO(
        GF_Read,
        pixelWindow.leftCol, pixelWindow.topRow,
        width, height,
        buffer.data(),
        width, height,
        GDT_Float32,
        0, 0
    );

    if (err != CE_None) {
        std::cerr << "[ERROR] Failed to read raster data" << std::endl;
        return false;
    }

	// Step 3: Convert 1D buffer to 2D depths matrix
    depths.assign(height, std::vector<float>(width));
    for (int r = 0; r < height; ++r) { 
        std::memcpy(depths[r].data(), buffer.data() + r * width, sizeof(float) * width);
    }

	// Step 4: Stroe geographic resolution
    latStepGeo = std::abs(geoTransform[5]);
    lonStepGeo = std::abs(geoTransform[1]);

    return true;
}


float GebcoLoader::GetDepthAt(double lon, double lat) const {
    if (!IsOpen()) return 0.0f;
    double px, py; GeoToPixelCont(lon, lat, px, py);
    const int col = static_cast<int>(std::floor(px));
    const int row = static_cast<int>(std::floor(py));
    if (col < 0 || col >= rasterWidth || row < 0 || row >= rasterHeight) return 0.0f;

    float depth = 0.0f;
    band->RasterIO(GF_Read, col, row, 1, 1, &depth, 1, 1, GDT_Float32, 0, 0);
    return depth;
}
