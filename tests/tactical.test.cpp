#include "geo/tactical.hpp"

#include <gtest/gtest.h>

#include <cmath>

namespace {

constexpr double kWgs84A = 6378137.0;
constexpr double kDeg = M_PI / 180.0;

using geo::Bearing;
using geo::Distance;
using geo::LatLon;

// HLR-GEO-030: a point due east of the bullseye on the equator reads as
// bearing 090 / range = a*(pi/180).
TEST(Bullseye, ToBullseyeOnEquator) {
  const auto fix = geo::to_bullseye(LatLon{0.0, 0.0}, LatLon{0.0, 1.0});
  EXPECT_NEAR(fix.bearing.degrees(), 90.0, 1e-9);
  EXPECT_NEAR(fix.range.meters(), kWgs84A * kDeg, 1e-3);
}

// HLR-GEO-031: bearing 090 / that range from the bullseye returns the point.
TEST(Bullseye, FromBullseyeOnEquator) {
  const auto p = geo::from_bullseye(LatLon{0.0, 0.0}, Bearing::from_degrees(90.0),
                                    Distance::from_meters(kWgs84A * kDeg));
  EXPECT_NEAR(p.lat_deg, 0.0, 1e-9);
  EXPECT_NEAR(p.lon_deg, 1.0, 1e-6);
}

// HLR-GEO-030/031: from_bullseye then to_bullseye round-trips.
TEST(Bullseye, RoundTrip) {
  const LatLon bull{36.2, 127.9};  // an arbitrary reference
  const auto brg = Bearing::from_degrees(215.0);
  const auto rng = Distance::from_nm(80.0);

  const auto p = geo::from_bullseye(bull, brg, rng);
  const auto fix = geo::to_bullseye(bull, p);
  EXPECT_NEAR(fix.bearing.degrees(), 215.0, 1e-9);
  EXPECT_NEAR(fix.range.nm(), 80.0, 1e-6);
}

// HLR-GEO-032: a target placed 135/200 from ownship reads back as bearing 135,
// range 200 NM (BRAA inverts the bullseye/forward construction).
TEST(Braa, BearingAndRange) {
  const LatLon ownship{37.46, 126.44};
  const auto target =
      geo::from_bullseye(ownship, Bearing::from_degrees(135.0), Distance::from_nm(200.0));
  const auto b = geo::braa(ownship, target);
  EXPECT_NEAR(b.bearing.degrees(), 135.0, 1e-9);  // SE
  EXPECT_NEAR(b.range.nm(), 200.0, 1e-6);
}

// HLR-GEO-033: aspect at the cardinal positions around a north-bound target.
TEST(Aspect, CardinalPositions) {
  const LatLon tgt{0.0, 0.0};
  const auto north = Bearing::from_degrees(0.0);  // target heading due north

  EXPECT_NEAR(geo::aspect_angle({1.0, 0.0}, tgt, north).angle_deg, 180.0, 1e-6);   // hot
  EXPECT_NEAR(geo::aspect_angle({-1.0, 0.0}, tgt, north).angle_deg, 0.0, 1e-6);    // stern

  const auto rightBeam = geo::aspect_angle({0.0, 1.0}, tgt, north);  // ownship east
  EXPECT_NEAR(rightBeam.angle_deg, 90.0, 1e-6);
  EXPECT_EQ(rightBeam.side, 'R');

  const auto leftBeam = geo::aspect_angle({0.0, -1.0}, tgt, north);  // ownship west
  EXPECT_NEAR(leftBeam.angle_deg, 90.0, 1e-6);
  EXPECT_EQ(leftBeam.side, 'L');
}

}  // namespace
