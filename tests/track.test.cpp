#include "geo/track.hpp"

#include <gtest/gtest.h>

#include <cmath>

#include "geo/geodesic.hpp"

namespace {

using geo::Distance;
using geo::LatLon;

// HLR-GEO-043: zero velocity leaves the position unchanged.
TEST(DeadReckon, ZeroVelocityHolds) {
  const LatLon start{37.0, 127.0};
  const auto p = geo::dead_reckon(start, 0.0, 0.0, 600.0);
  EXPECT_NEAR(p.lat_deg, start.lat_deg, 1e-9);
  EXPECT_NEAR(p.lon_deg, start.lon_deg, 1e-9);
}

// HLR-GEO-043: after dt the track has moved |v|*dt along bearing atan2(vE, vN).
TEST(DeadReckon, MovesAlongVelocity) {
  const LatLon start{37.0, 127.0};
  const double vE = 200.0, vN = 150.0, dt = 60.0;  // |v| = 250 m/s, 15 km in 60 s

  const auto p = geo::dead_reckon(start, vE, vN, dt);
  const auto arc = geo::inverse_ellipsoidal(start, p);

  EXPECT_NEAR(arc.distance.meters(), std::hypot(vE, vN) * dt, 1.0);
  const double bearing = std::atan2(vE, vN) * 180.0 / M_PI;  // 53.13 deg
  EXPECT_NEAR(arc.initial_bearing.degrees(), bearing, 1e-2);
}

}  // namespace
