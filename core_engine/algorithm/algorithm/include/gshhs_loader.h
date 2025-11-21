#pragma once
#include "common_types.h"
#include <vector>
#include <string>
#include <gdal_priv.h>
#include <ogrsf_frmts.h>

struct GSHHSPolygon {
    std::vector<GeoCoordinate> points;
    int level;  // 1=land, 2=lake, 3=island_in_lake, 4=pond_in_island

    bool Intersects(const BoundingBox& roi) const;
};

class GshhsLoader {
private:
    std::string filepath;
    GDALDataset* dataset;
    std::vector<GSHHSPolygon> polygons;

    bool ParseShapefile();

public:
    GshhsLoader(const std::string& filepath);
    ~GshhsLoader();

    bool Open();
    void Close();
    bool IsOpen() const { return dataset != nullptr; }

    std::vector<GSHHSPolygon> ExtractROI(const BoundingBox& roi) const;

    size_t GetPolygonCount() const { return polygons.size(); }
};