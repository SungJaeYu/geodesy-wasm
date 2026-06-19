#include "geo/tactical.hpp"

#include <cmath>

#include "geo/geodesic.hpp"

namespace geo {
namespace {

// Signed difference b - a wrapped to (-180, 180].
double wrap180(double deg) {
  double d = std::fmod(deg + 180.0, 360.0);
  if (d < 0.0) d += 360.0;
  return d - 180.0;
}

}  // namespace

PolarFix to_bullseye(LatLon bullseye, LatLon point) {
  const auto arc = inverse_ellipsoidal(bullseye, point);
  return PolarFix{arc.initial_bearing, arc.distance};
}

LatLon from_bullseye(LatLon bullseye, Bearing bearing, Distance range) {
  return forward_ellipsoidal(bullseye, bearing, range);
}

PolarFix braa(LatLon ownship, LatLon target) {
  const auto arc = inverse_ellipsoidal(ownship, target);
  return PolarFix{arc.initial_bearing, arc.distance};
}

Aspect aspect_angle(LatLon ownship, LatLon target, Bearing target_heading) {
  // Bearing from the target to ownship, relative to the target's nose.
  const auto los = inverse_ellipsoidal(target, ownship).initial_bearing;
  const double off = wrap180(los.degrees() - target_heading.degrees());
  const double angle = 180.0 - std::abs(off);

  char side = '\0';
  if (std::abs(off) > 1e-9 && std::abs(std::abs(off) - 180.0) > 1e-9) {
    side = off > 0.0 ? 'R' : 'L';
  }
  return Aspect{angle, side};
}

}  // namespace geo
