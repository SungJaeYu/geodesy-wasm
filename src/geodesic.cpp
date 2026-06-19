#include "geo/geodesic.hpp"

#include <GeographicLib/Geodesic.hpp>
#include <GeographicLib/GeodesicLine.hpp>
#include <GeographicLib/Rhumb.hpp>

#include <cmath>

namespace geo {

LatLon forward_ellipsoidal(LatLon start, Bearing bearing_true, Distance dist) {
  const GeographicLib::Geodesic& g = GeographicLib::Geodesic::WGS84();
  double lat2 = 0.0;
  double lon2 = 0.0;
  g.Direct(start.lat_deg, start.lon_deg, bearing_true.degrees(), dist.meters(),
           lat2, lon2);
  return LatLon{lat2, lon2};
}

LatLon forward_spherical(LatLon start, Bearing bearing_true, Distance dist) {
  // Mean radius of the WGS84 ellipsoid: (2a + b) / 3.
  constexpr double kMeanR = 6371008.7714;

  const double ang = dist.meters() / kMeanR;  // angular distance (radians)
  const double brg = bearing_true.radians();
  const double lat1 = start.lat_deg * M_PI / 180.0;
  const double lon1 = start.lon_deg * M_PI / 180.0;

  const double lat2 = std::asin(std::sin(lat1) * std::cos(ang) +
                                std::cos(lat1) * std::sin(ang) * std::cos(brg));
  const double lon2 =
      lon1 + std::atan2(std::sin(brg) * std::sin(ang) * std::cos(lat1),
                        std::cos(ang) - std::sin(lat1) * std::sin(lat2));

  return LatLon{lat2 * 180.0 / M_PI, lon2 * 180.0 / M_PI};
}

GeodesicArc inverse_ellipsoidal(LatLon p1, LatLon p2) {
  const GeographicLib::Geodesic& g = GeographicLib::Geodesic::WGS84();
  double s12 = 0.0, azi1 = 0.0, azi2 = 0.0;
  g.Inverse(p1.lat_deg, p1.lon_deg, p2.lat_deg, p2.lon_deg, s12, azi1, azi2);
  return GeodesicArc{Bearing::from_degrees(azi1), Bearing::from_degrees(azi2),
                     Distance::from_meters(s12)};
}

GeodesicArc inverse_spherical(LatLon p1, LatLon p2) {
  constexpr double kMeanR = 6371008.7714;
  const double lat1 = p1.lat_deg * M_PI / 180.0;
  const double lat2 = p2.lat_deg * M_PI / 180.0;
  const double dlon = (p2.lon_deg - p1.lon_deg) * M_PI / 180.0;

  // Haversine central angle and great-circle bearings.
  const double sdlat = std::sin((lat2 - lat1) / 2.0);
  const double sdlon = std::sin(dlon / 2.0);
  const double a = sdlat * sdlat +
                   std::cos(lat1) * std::cos(lat2) * sdlon * sdlon;
  const double ang = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));

  const double y1 = std::sin(dlon) * std::cos(lat2);
  const double x1 = std::cos(lat1) * std::sin(lat2) -
                    std::sin(lat1) * std::cos(lat2) * std::cos(dlon);
  const double azi1 = std::atan2(y1, x1) * 180.0 / M_PI;

  // Final bearing: initial bearing of the reverse arc, flipped 180 deg.
  const double y2 = std::sin(-dlon) * std::cos(lat1);
  const double x2 = std::cos(lat2) * std::sin(lat1) -
                    std::sin(lat2) * std::cos(lat1) * std::cos(-dlon);
  const double azi2 = std::atan2(y2, x2) * 180.0 / M_PI + 180.0;

  return GeodesicArc{Bearing::from_degrees(azi1), Bearing::from_degrees(azi2),
                     Distance::from_meters(ang * kMeanR)};
}

RhumbArc inverse_rhumb(LatLon p1, LatLon p2) {
  const GeographicLib::Rhumb& r = GeographicLib::Rhumb::WGS84();
  double s12 = 0.0, azi12 = 0.0;
  r.Inverse(p1.lat_deg, p1.lon_deg, p2.lat_deg, p2.lon_deg, s12, azi12);
  return RhumbArc{Bearing::from_degrees(azi12), Distance::from_meters(s12)};
}

LatLon intermediate(LatLon p1, LatLon p2, double fraction) {
  const GeographicLib::Geodesic& g = GeographicLib::Geodesic::WGS84();
  const GeographicLib::GeodesicLine line =
      g.InverseLine(p1.lat_deg, p1.lon_deg, p2.lat_deg, p2.lon_deg);
  double lat = 0.0, lon = 0.0;
  line.Position(fraction * line.Distance(), lat, lon);
  return LatLon{lat, lon};
}

LatLon midpoint(LatLon p1, LatLon p2) { return intermediate(p1, p2, 0.5); }

}  // namespace geo
