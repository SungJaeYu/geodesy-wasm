#include "geo/track.hpp"

#include <GeographicLib/LocalCartesian.hpp>

namespace geo {

LatLon dead_reckon(LatLon start, double v_east_mps, double v_north_mps,
                   double dt_seconds) {
  // Propagate in a local ENU frame centred at the start point, then convert the
  // displaced position back to geodetic. Equivalent to a Kalman predict step:
  // east/north <- east/north + velocity * dt.
  const GeographicLib::LocalCartesian enu(start.lat_deg, start.lon_deg, 0.0);
  const double east = v_east_mps * dt_seconds;
  const double north = v_north_mps * dt_seconds;
  double lat = 0.0, lon = 0.0, h = 0.0;
  enu.Reverse(east, north, 0.0, lat, lon, h);
  return LatLon{lat, lon};
}

}  // namespace geo
