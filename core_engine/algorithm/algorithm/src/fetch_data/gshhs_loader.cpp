//gshhs_loader.cpp
#include "../../include/gshhs_loader.h"
#include <iostream>

GshhsLoader::GshhsLoader(const std::string& filepath)
    : filepath(filepath), dataset(nullptr) {
}

GshhsLoader::~GshhsLoader() {
    Close();
}

bool GshhsLoader::Open() {
    dataset = (GDALDataset*)GDALOpenEx(
        filepath.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);

    if (!dataset) {
        std::cerr << "[ERROR] Failed to open GSHHS file: " << filepath << std::endl;
        return false;
    }

    if (!ParseShapefile()) {
        Close();
        return false;
    }

#ifdef _DEBUG
    std::cout << "[DEBUG][GshhsLoader] Opened: " << filepath << "\n"
        << "  - Polygon count : " << polygons.size() << "\n";
#endif

    return true;
}

void GshhsLoader::Close() {
    if (dataset) {
        GDALClose(dataset);
        dataset = nullptr;
    }
    polygons.clear();
}

bool GshhsLoader::ParseShapefile() {
    OGRLayer* layer = dataset->GetLayer(0);
    if (!layer) {
        std::cerr << "[ERROR] Could not find layer in GSHHS file" << std::endl;
        return false;
    }

    layer->ResetReading();
    OGRFeature* feature;

    while ((feature = layer->GetNextFeature()) != nullptr) {
        OGRGeometry* geometry = feature->GetGeometryRef();

        if (geometry && wkbFlatten(geometry->getGeometryType()) == wkbPolygon) {
            OGRPolygon* polygon = (OGRPolygon*)geometry;
            OGRLinearRing* ring = polygon->getExteriorRing();

            if (ring) {
                GSHHSPolygon poly;

                int levelIdx = feature->GetFieldIndex("level");
                poly.level = (levelIdx >= 0) ? feature->GetFieldAsInteger(levelIdx) : 1;

                for (int i = 0; i < ring->getNumPoints(); ++i) {
                    poly.points.emplace_back(ring->getY(i), ring->getX(i));
                }

                polygons.push_back(std::move(poly));
            }
        }

        OGRFeature::DestroyFeature(feature);
    }

    return !polygons.empty();
}

std::vector<GSHHSPolygon> GshhsLoader::ExtractROI(const BoundingBox& roi) const {
    std::vector<GSHHSPolygon> result;
    result.reserve(polygons.size());

    for (const auto& poly : polygons) {
        if (poly.Intersects(roi)) {
            result.push_back(poly);
        }
    }

    //std::cout << "[INFO] Extracted " << result.size() << " polygons in ROI" << std::endl;
    return result;
}

bool GSHHSPolygon::Intersects(const BoundingBox& roi) const {
    if (points.empty()) return false;

    double minLat = points[0].latitude, maxLat = points[0].latitude;
    double minLon = points[0].longitude, maxLon = points[0].longitude;

    for (const auto& pt : points) {
        minLat = std::min(minLat, pt.latitude);
        maxLat = std::max(maxLat, pt.latitude);
        minLon = std::min(minLon, pt.longitude);
        maxLon = std::max(maxLon, pt.longitude);
    }

    return !(maxLat < roi.minLat || minLat > roi.maxLat ||
        maxLon < roi.minLon || minLon > roi.maxLon);
}