#pragma once

#include "geo/types.hpp"

namespace geo {

// A position expressed relative to a reference point: true bearing + range
// from the reference to the point. Used for both bullseye and BRAA calls.
struct PolarFix {
  Bearing bearing;
  Distance range;
};

// --- Bullseye -----------------------------------------------------------------
// A bullseye is an agreed reference point; positions are exchanged as a
// bearing+range FROM the bullseye, and converted back to absolute coordinates.
// HLR-GEO-030 to_bullseye   HLR-GEO-031 from_bullseye
PolarFix to_bullseye(LatLon bullseye, LatLon point);
LatLon from_bullseye(LatLon bullseye, Bearing bearing, Distance range);

// --- BRAA ---------------------------------------------------------------------
// Target relative to ownship: true bearing + geodesic ground range from ownship
// to target. (Altitude is a passthrough; slant range lives in radar.hpp.)
// HLR-GEO-032 braa
PolarFix braa(LatLon ownship, LatLon target);

// Target aspect angle: 0 = ownship on the target's tail (stern), 180 = head-on
// (hot). `side` is 'L'/'R' for which side of the target's nose ownship lies on,
// or '\0' when aspect is 0 or 180. Needs the target's heading.
// HLR-GEO-033 aspect_angle
struct Aspect {
  double angle_deg;  // 0..180
  char side;         // 'L', 'R', or '\0'
};
Aspect aspect_angle(LatLon ownship, LatLon target, Bearing target_heading);

}  // namespace geo
