#include "geo/radar.hpp"

#include <gtest/gtest.h>

#include <cmath>

#include "geo/geodesic.hpp"

namespace {

using geo::Bearing;
using geo::Distance;
using geo::LatLon;

// HLR-GEO-040: directly overhead, the slant range is just the height difference.
TEST(Slant, VerticalSeparation) {
  const auto s = geo::slant_range(LatLon{37.0, 127.0}, Distance::from_meters(0.0),
                                  LatLon{37.0, 127.0}, Distance::from_feet(30000.0));
  EXPECT_NEAR(s.feet(), 30000.0, 1e-3);
}

// HLR-GEO-041: zero ground range -> slant equals the vertical separation.
TEST(SlantGround, ZeroGroundIsVertical) {
  const auto s = geo::slant_from_ground(Distance::from_meters(0.0),
                                        Distance::from_meters(1000.0),
                                        Distance::from_meters(0.0));
  EXPECT_NEAR(s.meters(), 1000.0, 1e-6);
}

// HLR-GEO-041: slant_from_ground and ground_from_slant invert each other.
TEST(SlantGround, RoundTrip) {
  const auto h1 = Distance::from_feet(25000.0);
  const auto h2 = Distance::from_feet(5000.0);
  const auto ground = Distance::from_nm(120.0);

  const auto slant = geo::slant_from_ground(ground, h1, h2);
  EXPECT_GT(slant.meters(), ground.meters());  // slant is the hypotenuse-like leg
  const auto back = geo::ground_from_slant(slant, h1, h2);
  EXPECT_NEAR(back.nm(), 120.0, 1e-6);
}

// HLR-GEO-042: radar horizon ~ 4.12*sqrt(h[m]) km with the 4/3-earth model;
// zero height -> zero horizon.
TEST(Horizon, FourThirdsApproximation) {
  EXPECT_NEAR(geo::radar_horizon(Distance::from_meters(0.0)).meters(), 0.0, 1e-9);
  const auto d = geo::radar_horizon(Distance::from_meters(100.0), true);
  EXPECT_NEAR(d.km(), 4.122 * std::sqrt(100.0), 0.05);  // ~41.2 km
}

// HLR-GEO-042: line of sight holds inside the combined horizon and fails well
// beyond it. Points are placed at a known ground range via the forward geodesic.
TEST(LineOfSight, InsideAndBeyondHorizon) {
  const LatLon a{37.0, 127.0};
  const auto h = Distance::from_meters(100.0);  // horizon ~41 km each => ~82 km combined
  const auto east = Bearing::from_degrees(90.0);

  const auto near = geo::forward_ellipsoidal(a, east, Distance::from_km(50.0));
  const auto far = geo::forward_ellipsoidal(a, east, Distance::from_km(150.0));
  EXPECT_TRUE(geo::has_line_of_sight(a, h, near, h));
  EXPECT_FALSE(geo::has_line_of_sight(a, h, far, h));
}

}  // namespace
