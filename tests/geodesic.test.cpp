#include "geo/geodesic.hpp"

#include <gtest/gtest.h>

#include <cmath>

namespace {

// WGS84 equatorial radius and the mean radius used by the spherical model.
constexpr double kWgs84A = 6378137.0;
constexpr double kMeanR = 6371008.7714;
constexpr double kDeg = M_PI / 180.0;

using geo::Bearing;
using geo::Distance;
using geo::LatLon;

// HLR-GEO-001: due east along the equator on the ellipsoid advances longitude
// by exactly arc / a. Picking arc = a*(pi/180) gives exactly 1 degree, a
// known-answer independent of GeographicLib's internals.
TEST(ForwardGeodesic, EllipsoidalDueEastAlongEquator) {
  const auto dest = geo::forward_ellipsoidal(
      LatLon{0.0, 0.0}, Bearing::from_degrees(90.0),
      Distance::from_meters(kWgs84A * kDeg));

  EXPECT_NEAR(dest.lat_deg, 0.0, 1e-9);
  EXPECT_NEAR(dest.lon_deg, 1.0, 1e-6);
}

// HLR-GEO-002: due north from the equator on the sphere advances latitude by
// exactly arc / R. arc = R*(pi/180) gives exactly 1 degree.
TEST(ForwardGeodesic, SphericalDueNorthFromEquator) {
  const auto dest = geo::forward_spherical(
      LatLon{0.0, 0.0}, Bearing::from_degrees(0.0),
      Distance::from_meters(kMeanR * kDeg));

  EXPECT_NEAR(dest.lat_deg, 1.0, 1e-9);
  EXPECT_NEAR(dest.lon_deg, 0.0, 1e-9);
}

// The whole point of keeping both models: they must give measurably different
// answers for the same problem (here, ~1 km after 1000 km due east at mid-lat).
TEST(ForwardGeodesic, EllipsoidalAndSphericalDiffer) {
  const LatLon start{37.0, 127.0};
  const auto brg = Bearing::from_degrees(90.0);
  const auto dist = Distance::from_km(1000.0);

  const auto e = geo::forward_ellipsoidal(start, brg, dist);
  const auto s = geo::forward_spherical(start, brg, dist);

  const double dlat = e.lat_deg - s.lat_deg;
  const double dlon = e.lon_deg - s.lon_deg;
  const double sep = std::hypot(dlat, dlon);
  EXPECT_GT(sep, 1e-3);   // models genuinely diverge
  EXPECT_LT(sep, 1.0);    // but not absurdly (sanity bound)
}

// HLR-GEO-003: inverse on the equator returns arc = a*(Δlon in rad) and a due
// east bearing at both ends.
TEST(InverseGeodesic, EllipsoidalAlongEquator) {
  const auto arc = geo::inverse_ellipsoidal(LatLon{0.0, 0.0}, LatLon{0.0, 1.0});
  EXPECT_NEAR(arc.distance.meters(), kWgs84A * kDeg, 1e-3);
  EXPECT_NEAR(arc.initial_bearing.degrees(), 90.0, 1e-9);
  EXPECT_NEAR(arc.final_bearing.degrees(), 90.0, 1e-9);
}

// HLR-GEO-004: spherical inverse along a meridian returns R*(Δlat in rad), due
// north.
TEST(InverseGeodesic, SphericalAlongMeridian) {
  const auto arc = geo::inverse_spherical(LatLon{0.0, 0.0}, LatLon{1.0, 0.0});
  EXPECT_NEAR(arc.distance.meters(), kMeanR * kDeg, 1e-3);
  EXPECT_NEAR(arc.initial_bearing.degrees(), 0.0, 1e-9);
}

// HLR-GEO-005: rhumb along the equator coincides with the geodesic there.
TEST(InverseGeodesic, RhumbAlongEquator) {
  const auto arc = geo::inverse_rhumb(LatLon{0.0, 0.0}, LatLon{0.0, 1.0});
  EXPECT_NEAR(arc.distance.meters(), kWgs84A * kDeg, 1e-3);
  EXPECT_NEAR(arc.bearing.degrees(), 90.0, 1e-9);
}

// HLR-GEO-001/003: forward then inverse must round-trip back to the same
// distance and initial bearing.
TEST(Geodesic, ForwardInverseRoundTrip) {
  const LatLon start{37.4602, 126.4407};  // ~Incheon
  const auto brg = Bearing::from_degrees(73.0);
  const auto dist = Distance::from_nm(250.0);

  const auto dest = geo::forward_ellipsoidal(start, brg, dist);
  const auto arc = geo::inverse_ellipsoidal(start, dest);

  EXPECT_NEAR(arc.distance.nm(), 250.0, 1e-6);
  EXPECT_NEAR(arc.initial_bearing.degrees(), 73.0, 1e-9);
}

// HLR-GEO-006/007: the midpoint of an equatorial arc sits halfway along it.
TEST(Geodesic, IntermediateAndMidpointOnEquator) {
  const auto half = geo::intermediate(LatLon{0.0, 0.0}, LatLon{0.0, 2.0}, 0.5);
  EXPECT_NEAR(half.lat_deg, 0.0, 1e-9);
  EXPECT_NEAR(half.lon_deg, 1.0, 1e-6);

  const auto mid = geo::midpoint(LatLon{0.0, 0.0}, LatLon{0.0, 2.0});
  EXPECT_NEAR(mid.lat_deg, 0.0, 1e-9);
  EXPECT_NEAR(mid.lon_deg, 1.0, 1e-6);
}

}  // namespace
