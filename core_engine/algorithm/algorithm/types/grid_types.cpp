#include "grid_types.h"
#include <iostream>
#include <cmath>
#include <algorithm>

// ===== GridResolution Implementation =====
std::pair<int, int> GridResolution::CalculateBlockSize(
    const BoundingBox& roi,
    double targetCellSizeKm)
{
    using namespace GEBCOConstants;

    double avgLat = (roi.minLat + roi.maxLat) / 2.0;
    const double kmPerPixLon = KM_PER_PIXEL_LAT * std::cos(avgLat * PI / 180.0);

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

// ===== NavigableGrid Implementation =====
NavigableGrid::NavigableGrid()
    : rows_(0), cols_(0), cellSizeLat_(0), cellSizeLon_(0), mapper_() {
}

NavigableGrid::NavigableGrid(const BoundingBox& bounds, int rows, int cols)
    : geoBounds_(bounds), rows_(rows), cols_(cols), mapper_(bounds, rows, cols)
{
    grid_.resize(rows_, std::vector<CellType>(cols_, CellType::UNKNOWN));

    cellSizeLat_ = bounds.Height() / rows_;
    cellSizeLon_ = bounds.Width() / cols_;

#ifdef _DEBUG
    std::cout << "  - Cell Size (approx): " << cellSizeLat_ << "° x " << cellSizeLon_ << "°\n";
#endif
}

void NavigableGrid::Reset(const BoundingBox& bounds, int rows, int cols) {
    geoBounds_ = bounds;
    rows_ = rows;
    cols_ = cols;
    mapper_.Reset(bounds, rows, cols);
    grid_.resize(rows_, std::vector<CellType>(cols_, CellType::UNKNOWN));
    
    cellSizeLat_ = bounds.Height() / rows_;
    cellSizeLon_ = bounds.Width() / cols_;
}

CellType NavigableGrid::GetCellType(int row, int col) const {
    if (!IsValid(row, col)) return CellType::UNKNOWN;
    return grid_[row][col];
}

void NavigableGrid::SetCellType(int row, int col, CellType type) {
    if (IsValid(row, col)) {
        grid_[row][col] = type;
    }
}

bool NavigableGrid::IsNavigable(int row, int col) const {
    return IsValid(row, col) && grid_[row][col] == CellType::NAVIGABLE;
}

bool NavigableGrid::IsValid(int row, int col) const {
    return row >= 0 && row < rows_ && col >= 0 && col < cols_;
}

bool NavigableGrid::IsValid(const GridCoordinate& pos) const {
    return IsValid(pos.row, pos.col);
}

GridCoordinate NavigableGrid::GeoToGrid(const GeoCoordinate& geo) const {
    return mapper_.GeoToGrid(geo);
}

GeoCoordinate NavigableGrid::GridToGeo(const GridCoordinate& grid) const {
    return mapper_.GridToGeo(grid);
}

GeoCoordinate NavigableGrid::GridToGeo(int row, int col) const {
    return mapper_.GridToGeo(GridCoordinate(row, col));
}