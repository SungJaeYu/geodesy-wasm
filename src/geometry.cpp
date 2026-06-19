#include "geo/geometry.hpp"

#include <GeographicLib/Geodesic.hpp>
#include <GeographicLib/PolygonArea.hpp>

#include <cmath>

#include "geo/geodesic.hpp"

namespace geo {

bool sector_contains(LatLon center, LatLon p, Bearing brg_min, Bearing brg_max,
                     Distance range_min, Distance range_max) {
  const auto arc = inverse_ellipsoidal(center, p);
  const double rng = arc.distance.meters();
  if (rng < range_min.meters() || rng > range_max.meters()) return false;

  // Is the bearing within [brg_min, brg_max] sweeping clockwise?
  const double width = std::fmod(brg_max.degrees() - brg_min.degrees() + 360.0, 360.0);
  const double off = std::fmod(arc.initial_bearing.degrees() - brg_min.degrees() + 360.0, 360.0);
  return off <= width;
}

PolygonMeasure polygon_area(const std::vector<LatLon>& vertices) {
  GeographicLib::PolygonArea poly(GeographicLib::Geodesic::WGS84());
  for (const LatLon& v : vertices) poly.AddPoint(v.lat_deg, v.lon_deg);
  double perimeter = 0.0, area = 0.0;
  poly.Compute(false, true, perimeter, area);
  return {std::abs(area), perimeter};
}

LatLon polygon_centroid(const std::vector<LatLon>& vertices) {
  if (vertices.empty()) return LatLon{0.0, 0.0};
  double lat = 0.0, lon = 0.0;
  for (const LatLon& v : vertices) {
    lat += v.lat_deg;
    lon += v.lon_deg;
  }
  const double n = static_cast<double>(vertices.size());
  return LatLon{lat / n, lon / n};
}

bool point_in_polygon(const std::vector<LatLon>& vertices, LatLon p) {
  // Planar lat/lon ray casting (valid away from poles / antimeridian).
  bool inside = false;
  const std::size_t n = vertices.size();
  for (std::size_t i = 0, j = n - 1; i < n; j = i++) {
    const double yi = vertices[i].lat_deg, xi = vertices[i].lon_deg;
    const double yj = vertices[j].lat_deg, xj = vertices[j].lon_deg;
    const bool crosses = (yi > p.lat_deg) != (yj > p.lat_deg);
    if (crosses &&
        p.lon_deg < (xj - xi) * (p.lat_deg - yi) / (yj - yi) + xi) {
      inside = !inside;
    }
  }
  return inside;
}

}  // namespace geo
