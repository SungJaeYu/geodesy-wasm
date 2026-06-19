#pragma once

#include "geo/types.hpp"

namespace geo {

// Intersection of two geodesics, each defined by a point + initial true bearing
// (GeographicLib::Intersect — the crossing closest to the two start points).
// HLR-GEO-050
LatLon radial_intersection(LatLon p1, Bearing bearing1, LatLon p2,
                           Bearing bearing2);

// Intersection of two range rings (spherical model): points at `r1` from `p1`
// and `r2` from `p2`. 0, 1 (tangent), or 2 solutions; for count==2 both `a` and
// `b` are filled, for count<2 the unused point mirrors `a`.
// HLR-GEO-051
struct RangeRangeFix {
  int count;
  LatLon a;
  LatLon b;
};
RangeRangeFix range_range_intersection(LatLon p1, Distance r1, LatLon p2,
                                       Distance r2);

// Cross-track distance (signed: positive when p3 is right of the leg) and
// along-track distance of p3 relative to the great-circle leg p1->p2, plus the
// closest point on that leg (spherical model). HLR-GEO-052
Distance cross_track_distance(LatLon p1, LatLon p2, LatLon p3);
Distance along_track_distance(LatLon p1, LatLon p2, LatLon p3);
LatLon closest_point_on_leg(LatLon p1, LatLon p2, LatLon p3);

}  // namespace geo
