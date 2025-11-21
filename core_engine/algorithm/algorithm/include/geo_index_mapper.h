#pragma once
#include "common_types.h"
#include <algorithm>

class GeoIndexMapper {
public:
    GeoIndexMapper()
        : rows_(0), cols_(0),
        cellSizeLat_(0.0), cellSizeLon_(0.0) {
    }

    GeoIndexMapper(const BoundingBox& bounds, int rows, int cols)
        : bounds_(bounds), rows_(rows), cols_(cols)
    {
        Reset(bounds, rows, cols);
    }

    void Reset(const BoundingBox& bounds, int rows, int cols) {
        bounds_ = bounds;
        rows_ = rows;
        cols_ = cols;

        cellSizeLat_ = bounds_.Height() / static_cast<double>(rows_);
        cellSizeLon_ = bounds_.Width() / static_cast<double>(cols_);
    }

    GridCoordinate GeoToGrid(const GeoCoordinate& geo) const {
		// lat: top to down, row index
        int row = static_cast<int>((bounds_.maxLat - geo.latitude) / cellSizeLat_);

		// lon: left to right, column index
        int col = static_cast<int>((geo.longitude - bounds_.minLon) / cellSizeLon_);

        row = std::clamp(row, 0, rows_ - 1);
        col = std::clamp(col, 0, cols_ - 1);

        return GridCoordinate(row, col);
    }

    GeoCoordinate GridToGeo(const GridCoordinate& idx) const {
        double lat = bounds_.maxLat - (idx.row + 0.5) * cellSizeLat_;
        double lon = bounds_.minLon + (idx.col + 0.5) * cellSizeLon_;
        return GeoCoordinate(lat, lon);
    }

    const BoundingBox& Bounds() const { return bounds_; }
    int Rows() const { return rows_; }
    int Cols() const { return cols_; }
    double CellSizeLat() const { return cellSizeLat_; }
    double CellSizeLon() const { return cellSizeLon_; }

private:
    BoundingBox bounds_;
    int rows_;
    int cols_;
    double cellSizeLat_;
    double cellSizeLon_;
};
