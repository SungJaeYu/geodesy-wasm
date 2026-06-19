#include "geo/intersection.hpp"

#include <gtest/gtest.h>

#include <cmath>

#include "geo/geodesic.hpp"

namespace {

constexpr double kMeanR = 6371008.7714;
constexpr double kDeg = M_PI / 180.0;

using geo::Bearing;
using geo::Distance;
using geo::LatLon;

// HLR-GEO-050: the equator (from (0,0) heading east) and the meridian lon=10
// (from (0,10) heading north) cross at (0,10).
TEST(Intersection, RadialOnEquatorAndMeridian) {
  const auto x = geo::radial_intersection(LatLon{0.0, 0.0}, Bearing::from_degrees(90.0),
                                          LatLon{0.0, 10.0}, Bearing::from_degrees(0.0));
  EXPECT_NEAR(x.lat_deg, 0.0, 1e-6);
  EXPECT_NEAR(x.lon_deg, 10.0, 1e-6);
}

// HLR-GEO-051: two rings centred at (0,0) and (0,10) sized to pass through a
// known target return that target as one of the two solutions.
TEST(Intersection, RangeRangeRecoversTarget) {
  const LatLon p1{0.0, 0.0}, p2{0.0, 10.0}, target{1.0, 5.0};
  const auto r1 = geo::inverse_spherical(p1, target).distance;
  const auto r2 = geo::inverse_spherical(p2, target).distance;

  const auto fix = geo::range_range_intersection(p1, r1, p2, r2);
  ASSERT_EQ(fix.count, 2);
  // One solution is the target (1,5), the other its mirror (-1,5).
  const auto matches = [&](const LatLon& s) {
    return std::abs(s.lat_deg - 1.0) < 1e-6 && std::abs(s.lon_deg - 5.0) < 1e-6;
  };
  EXPECT_TRUE(matches(fix.a) || matches(fix.b));
}

// HLR-GEO-052: a point 1 deg north of the eastbound equator leg is one degree
// of arc off-track (to the left => negative) with its foot at (0,5).
TEST(Intersection, CrossAndAlongTrack) {
  const LatLon p1{0.0, 0.0}, p2{0.0, 10.0}, p3{1.0, 5.0};

  const auto xt = geo::cross_track_distance(p1, p2, p3);
  EXPECT_NEAR(std::abs(xt.meters()), kMeanR * kDeg, 1.0);
  EXPECT_LT(xt.meters(), 0.0);  // left of an eastbound leg

  const auto at = geo::along_track_distance(p1, p2, p3);
  EXPECT_NEAR(at.meters(), kMeanR * 5.0 * kDeg, 1.0);

  const auto foot = geo::closest_point_on_leg(p1, p2, p3);
  EXPECT_NEAR(foot.lat_deg, 0.0, 1e-6);
  EXPECT_NEAR(foot.lon_deg, 5.0, 1e-6);
}

}  // namespace
