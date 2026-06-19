#include "geo/intersection.hpp"

#include <GeographicLib/Geodesic.hpp>
#include <GeographicLib/Intersect.hpp>

#include <algorithm>
#include <cmath>

#include "geo/geodesic.hpp"

namespace geo {
namespace {
constexpr double kMeanR = 6371008.7714;
}

LatLon radial_intersection(LatLon p1, Bearing bearing1, LatLon p2,
                           Bearing bearing2) {
  const GeographicLib::Geodesic& g = GeographicLib::Geodesic::WGS84();
  GeographicLib::Intersect intersect(g);
  const auto pt = intersect.Closest(p1.lat_deg, p1.lon_deg, bearing1.degrees(),
                                    p2.lat_deg, p2.lon_deg, bearing2.degrees());
  double lat = 0.0, lon = 0.0;
  g.Direct(p1.lat_deg, p1.lon_deg, bearing1.degrees(), pt.first, lat, lon);
  return LatLon{lat, lon};
}

RangeRangeFix range_range_intersection(LatLon p1, Distance r1, LatLon p2,
                                       Distance r2) {
  // Spherical two-circle intersection via the law of cosines in the triangle
  // p1-p2-X (sides d, theta1, theta2; solve the angle at p1).
  const auto arc = inverse_spherical(p1, p2);
  const double d = arc.distance.meters() / kMeanR;
  const double th1 = r1.meters() / kMeanR;
  const double th2 = r2.meters() / kMeanR;
  const double denom = std::sin(th1) * std::sin(d);
  if (std::abs(denom) < 1e-15) return {0, p1, p1};

  double cosA = (std::cos(th2) - std::cos(th1) * std::cos(d)) / denom;
  if (cosA > 1.0 + 1e-9 || cosA < -1.0 - 1e-9) return {0, p1, p1};
  cosA = std::min(1.0, std::max(-1.0, cosA));

  const double A = std::acos(cosA) * 180.0 / M_PI;
  const double beta = arc.initial_bearing.degrees();
  const auto a = forward_spherical(p1, Bearing::from_degrees(beta + A), r1);
  const auto b = forward_spherical(p1, Bearing::from_degrees(beta - A), r1);
  return {A < 1e-9 ? 1 : 2, a, b};
}

Distance cross_track_distance(LatLon p1, LatLon p2, LatLon p3) {
  const auto a13 = inverse_spherical(p1, p3);
  const double d13 = a13.distance.meters() / kMeanR;
  const double t13 = a13.initial_bearing.radians();
  const double t12 = inverse_spherical(p1, p2).initial_bearing.radians();
  const double dxt = std::asin(std::sin(d13) * std::sin(t13 - t12));
  return Distance::from_meters(dxt * kMeanR);
}

Distance along_track_distance(LatLon p1, LatLon p2, LatLon p3) {
  const double d13 = inverse_spherical(p1, p3).distance.meters() / kMeanR;
  const double dxt = cross_track_distance(p1, p2, p3).meters() / kMeanR;
  const double dat = std::acos(std::cos(d13) / std::cos(dxt));
  return Distance::from_meters(dat * kMeanR);
}

LatLon closest_point_on_leg(LatLon p1, LatLon p2, LatLon p3) {
  const auto t12 = inverse_spherical(p1, p2).initial_bearing;
  return forward_spherical(p1, t12, along_track_distance(p1, p2, p3));
}

}  // namespace geo
