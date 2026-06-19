#pragma once

#include "geo/types.hpp"

namespace geo {

// Constant-velocity dead reckoning in a local ENU frame centred at `start`:
// the position is displaced by velocity * dt and converted back to geodetic
// (GeographicLib::LocalCartesian). Velocity is given as east/north components
// in m/s; vertical motion is ignored (horizontal track).
//
// This is structurally identical to a Kalman filter's predict step
// (x <- x + v*dt) — the same propagation a tracker applies between updates.
// HLR-GEO-043
LatLon dead_reckon(LatLon start, double v_east_mps, double v_north_mps,
                   double dt_seconds);

}  // namespace geo
