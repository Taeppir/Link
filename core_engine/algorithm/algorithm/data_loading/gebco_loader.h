#pragma once
#include "../types/geo_types.h"
#include <string>
#include <vector>
#include <gdal_priv.h>

// ===== 기존 gebco_loader.h 내용 100% 그대로 =====

struct PixelWindow {
    int leftCol, topRow, rightCol, bottomRow;
    int width() const { return rightCol - leftCol + 1; }
    int height() const { return bottomRow - topRow + 1; }
};

class GebcoLoader {
private:
    std::string filepath;
    GDALDataset* dataset;
    GDALRasterBand* band;

    double geoTransform[6];
    double invGeoTransform[6];
    int rasterWidth;
    int rasterHeight;

    bool InitInverseGeoTransform();

    void GeoToPixelCont(double lon, double lat, double& pixelX, double& pixelY) const;
    void PixelCornerToGeo(int col, int row, double& lon, double& lat) const;
    void GeoToPixelIndexTopLeft(double lon, double lat, int& col, int& row) const;
    void GeoToPixelIndexBottomRight(double lon, double lat, int& col, int& row) const;

    bool ExpandROIWithPixelMargin(
        const BoundingBox& baseROI,
        int marginPixel,
        PixelWindow& pixelWindow,
        BoundingBox& expandedROI
    ) const;

public:
    GebcoLoader(const std::string& filepath);
    ~GebcoLoader();

    bool Open();
    void Close();
    bool IsOpen() const { return dataset != nullptr; }

    bool ExtractROI(
        const BoundingBox& baseROI,
        std::vector<std::vector<float>>& depths,
        double& latStepDegPerPixel,
        double& lonStepDegPerPixel,
        int pixelMargin,
        BoundingBox& expandedROI
    ) const;

    float GetDepthAt(double lon, double lat) const;

    int GetWidth() const { return rasterWidth; }
    int GetHeight() const { return rasterHeight; }
};