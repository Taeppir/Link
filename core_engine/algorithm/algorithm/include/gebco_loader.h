#pragma once
#include "common_types.h"
#include <string>
#include <vector>
#include <gdal_priv.h>

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
    double invGeoTransform[6]; // 역변환 행렬
    int rasterWidth;
    int rasterHeight;

    bool InitInverseGeoTransform();

    // 지리(lon,lat) → 픽셀 '코너 그리드' 연속 좌표(실수)
    void GeoToPixelCont(double lon, double lat, double& pixelX, double& pixelY) const;

    // 픽셀 '코너'(좌상 모서리) → 지리(lon,lat)
    void PixelCornerToGeo(int col, int row, double& lon, double& lat) const;

    // 방위 기반 스냅: 북서(NW)=좌/상=floor, 남동(SE)=우/하=ceil-1
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
		const BoundingBox& baseROI,                 // in: base ROI without margin
		std::vector<std::vector<float>>& depths,    // out: 2D depth array
		double& latStepDegPerPixel,                         // out: latitude step in degrees
		double& lonStepDegPerPixel,                         // out: longitude step in degrees
		int pixelMargin,                            // in: pixel margin to add around base ROI
		BoundingBox& expandedROI                    // out: actual ROI with margin
    ) const;

    //float GetDepthAt(double lat, double lon) const;
    float GetDepthAt(double lon, double lat) const;

    int GetWidth() const { return rasterWidth; }
    int GetHeight() const { return rasterHeight; }
};