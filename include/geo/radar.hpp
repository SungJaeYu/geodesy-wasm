#pragma once

#include "geo/types.hpp"

namespace geo {

// --- Slant range --------------------------------------------------------------
// Exact straight-line 3D distance between two positions at altitude, via ECEF
// (GeographicLib::Geocentric). HLR-GEO-040
Distance slant_range(LatLon a, Distance height_a, LatLon b, Distance height_b);

// Slant <-> ground range using a spherical-earth radar triangle (law of cosines
// about the earth's centre), when only a ground range and the two heights are
// known. `ground` is the surface (arc) range. HLR-GEO-041
Distance slant_from_ground(Distance ground, Distance height_a, Distance height_b);
Distance ground_from_slant(Distance slant, Distance height_a, Distance height_b);

// --- Radar horizon / line of sight -------------------------------------------
// Surface distance to the radar horizon for an antenna/target at `height`.
// `four_thirds` applies the standard 4/3-earth-radius refraction approximation.
// HLR-GEO-042
Distance radar_horizon(Distance height, bool four_thirds = true);

// Maximum line-of-sight range between two heights = sum of their horizons.
Distance radar_horizon_range(Distance height_a, Distance height_b,
                             bool four_thirds = true);

// True if `b` (at height_b) is within line of sight of `a` (at height_a): the
// geodesic ground range does not exceed the combined radar horizon.
bool has_line_of_sight(LatLon a, Distance height_a, LatLon b, Distance height_b,
                       bool four_thirds = true);

}  // namespace geo
