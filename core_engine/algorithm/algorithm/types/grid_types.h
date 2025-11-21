#pragma once
#include "geo_types.h"
#include "../utils/geo_calculations.h"
#include <vector>
#include <cstdint>
#include <algorithm>

// ===== Cell Type Definition =====
enum class CellType : uint8_t {
    UNKNOWN = 0,
    LAND = 1,
    SHALLOW = 2,
    NAVIGABLE = 3
};

// ===== Geo-Grid Coordinate Mapper =====
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
        int row = static_cast<int>((bounds_.maxLat - geo.latitude) / cellSizeLat_);
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

// ===== Grid Resolution Helper =====
namespace GEBCOConstants {
    constexpr double KM_PER_PIXEL_LAT = 0.463;  // 15 arc-second
}

struct GridResolution {
    double cellSizeKm;
    int blockLat;
    int blockLon;
    int rows;
    int cols;

    static std::pair<int, int> CalculateBlockSize(
        const BoundingBox& roi,
        double targetCellSizeKm);

    static GridResolution Calculate(
        const BoundingBox& roi,
        int srcHeight, int srcWidth,
        double targetCellSizeKm);
};

// ===== Navigable Grid (Pure Data Container) =====
class NavigableGrid {
public:
    NavigableGrid();
    NavigableGrid(const BoundingBox& bounds, int rows, int cols);

    void Reset(const BoundingBox& bounds, int rows, int cols);

    // Cell type access
    CellType GetCellType(int row, int col) const;
    void SetCellType(int row, int col, CellType type);
    bool IsNavigable(int row, int col) const;
    bool IsValid(int row, int col) const;
    bool IsValid(const GridCoordinate& pos) const;

    // Coordinate conversion
    GridCoordinate GeoToGrid(const GeoCoordinate& geo) const;
    GeoCoordinate GridToGeo(const GridCoordinate& grid) const;
    GeoCoordinate GridToGeo(int row, int col) const;

    // Grid properties
    int Rows() const { return rows_; }
    int Cols() const { return cols_; }
    const BoundingBox& Bounds() const { return geoBounds_; }
    const GeoIndexMapper& Mapper() const { return mapper_; }
    
    double CellSizeLat() const { return cellSizeLat_; }
    double CellSizeLon() const { return cellSizeLon_; }

    // Direct grid access (for builder)
    std::vector<std::vector<CellType>>& GetGridData() { return grid_; }
    const std::vector<std::vector<CellType>>& GetGridData() const { return grid_; }

private:
    BoundingBox geoBounds_;
    GeoIndexMapper mapper_;
    int rows_;
    int cols_;
    double cellSizeLat_;
    double cellSizeLon_;
    std::vector<std::vector<CellType>> grid_;
};