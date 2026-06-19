#pragma once

#include "geo/types.hpp"

namespace geo {

// Result of the inverse problem along a great-circle / geodesic: the two
// endpoints define an arc with a (generally varying) bearing, so both the
// initial and final bearing are reported alongside the distance.
struct GeodesicArc {
  Bearing initial_bearing;
  Bearing final_bearing;
  Distance distance;
};

// Result of the inverse problem along a rhumb line (loxodrome): a single
// constant bearing and the (longer) along-loxodrome distance.
struct RhumbArc {
  Bearing bearing;
  Distance distance;
};

// --- Forward (direct) problem -------------------------------------------------
// Given start, initial true bearing and distance, return the destination.
// HLR-GEO-001 forward_ellipsoidal   HLR-GEO-002 forward_spherical
LatLon forward_ellipsoidal(LatLon start, Bearing bearing_true, Distance dist);
LatLon forward_spherical(LatLon start, Bearing bearing_true, Distance dist);

// --- Inverse problem ----------------------------------------------------------
// Given two points, return bearing(s) and distance between them.
// HLR-GEO-003 inverse_ellipsoidal   HLR-GEO-004 inverse_spherical
// HLR-GEO-005 inverse_rhumb
GeodesicArc inverse_ellipsoidal(LatLon p1, LatLon p2);
GeodesicArc inverse_spherical(LatLon p1, LatLon p2);
RhumbArc inverse_rhumb(LatLon p1, LatLon p2);

// --- Intermediate points ------------------------------------------------------
// Point at `fraction` (0..1) of the geodesic from p1 to p2, and the midpoint.
// HLR-GEO-006 intermediate   HLR-GEO-007 midpoint
LatLon intermediate(LatLon p1, LatLon p2, double fraction);
LatLon midpoint(LatLon p1, LatLon p2);

}  // namespace geo
