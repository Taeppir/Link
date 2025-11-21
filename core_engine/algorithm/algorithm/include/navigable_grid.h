#pragma once
#include "common_types.h"
#include "gshhs_loader.h"
#include "geo_index_mapper.h"
#include <vector>
#include <gdal_priv.h>
#include <ogrsf_frmts.h>

namespace GEBCOConstants {
	// 위도 1도의 평균 거리
	// = {지구 적도 둘레} / 360도 = 약 111.319
	// 실제로는 110.574km(적도) ~ 111.694km(극) 범위이지만,
	// 대부분의 해양 항해 계산에서는 111km로 근사해도 충분
	constexpr double KM_PER_DEGREE_LAT = 111.0;    // ~111km per degree of latitude

	// GEBCO bathymetry data resolution
	constexpr double ARC_SECONDS = 15.0;           // GEBCO uses 15 arc-second grid
	constexpr double ARC_SECONDS_PER_DEGREE = 3600.0;  // 1 degree = 3600 arc-seconds


	// Derived constant: km per pixel at latitude
	constexpr double KM_PER_PIXEL_LAT = KM_PER_DEGREE_LAT * ARC_SECONDS / ARC_SECONDS_PER_DEGREE;
	// = 111.0 * 15.0 / 3600.0 = 111.0 / 240.0 = 약 0.4625 km
}

struct GridResolution {
	int rows;
	int cols;
	double cellSizeKm;
	int blockLat; // How many source pixels to group in the latitude direction (15" of GEBCO)
	int blockLon; // How many source pixels to group in the longtitude direction (15" of GEBCO)

	static std::pair<int, int> CalculateBlockSize(
		const BoundingBox& roi,
		double targetCellSizeKm
	);


	static GridResolution Calculate(
		const BoundingBox& roi,
		int srcHeight, int srcWidth,
		double targetCellSizeKm = 1.0
		);
};

class NavigableGrid {
private:
	std::vector<std::vector<CellType>> grid;
	BoundingBox geoBounds;
	int rows, cols;
	double cellSizeLat;
	double cellSizeLon;

	GeoIndexMapper mapper_;

public:
	NavigableGrid();
	NavigableGrid(const BoundingBox& roi, int rows, int cols);

	void BuildFromData(
		const BoundingBox&,
		const std::vector<std::vector<float>>& gebcoDepths,
		const std::vector<GSHHSPolygon>& gshhsPolygons,
		int blockLat, int blockLon
	);

	GeoCoordinate GridToGeo(int row, int col) const;
	GeoCoordinate GridToGeo(const GridCoordinate& grid) const;
	GridCoordinate GeoToGrid(const GeoCoordinate& geo) const;

	CellType GetCellType(int row, int col) const;
	bool IsNavigable(int row, int col) const;
	bool IsValid(int row, int col) const;
	bool IsValid(const GridCoordinate& pos) const;

	int GetRows() const { return rows; }
	int GetCols() const { return cols; }
	BoundingBox GetBounds() const { return geoBounds; }

	void SetAllCells(CellType cellType);

	void PrintStatistics() const;

private:
	// Step 1: GEBCO Downampling
	static std::vector<std::vector<float>> DownsampleDepths(
		const std::vector<std::vector<float>>& originalDepths,
		int targetRows,
		int targetCols,
		int blockLat, int blockLon
	);

	// Step 2: GSHHS Rasterization
	std::vector<std::vector<bool>> RasterizeGSHHS_GDAL(
		const std::vector<GSHHSPolygon>& polygons
	);

	// Step 3: Grid Masking
	void BuildMask(
		const std::vector<std::vector<float>>& downsampledDepths,
		const std::vector<std::vector<bool>>& landMask
	);
};