#pragma once

#include <vector>

#include "geo/types.hpp"

namespace geo {

// True if p lies within the sector from `center`: bearing within [brg_min,
// brg_max] measured clockwise (wrapping through 360) AND range within
// [range_min, range_max]. HLR-GEO-053
bool sector_contains(LatLon center, LatLon p, Bearing brg_min, Bearing brg_max,
                     Distance range_min, Distance range_max);

// Geodesic polygon area (m^2, non-negative) and perimeter (m), via
// GeographicLib::PolygonArea. HLR-GEO-054
struct PolygonMeasure {
  double area_m2;
  double perimeter_m;
};
PolygonMeasure polygon_area(const std::vector<LatLon>& vertices);

// Vertex-average centroid (approximate) and a planar lat/lon ray-cast
// point-in-polygon test (valid away from the poles and antimeridian).
// HLR-GEO-055
LatLon polygon_centroid(const std::vector<LatLon>& vertices);
bool point_in_polygon(const std::vector<LatLon>& vertices, LatLon p);

}  // namespace geo
