#include "geo/radar.hpp"

#include <GeographicLib/Geocentric.hpp>

#include <cmath>

#include "geo/geodesic.hpp"

namespace geo {
namespace {

// Mean WGS84 radius, used by the spherical radar-triangle and horizon models.
constexpr double kMeanR = 6371008.7714;

double effective_radius(bool four_thirds) {
  return four_thirds ? kMeanR * 4.0 / 3.0 : kMeanR;
}

}  // namespace

Distance slant_range(LatLon a, Distance height_a, LatLon b, Distance height_b) {
  const GeographicLib::Geocentric& earth = GeographicLib::Geocentric::WGS84();
  double x1, y1, z1, x2, y2, z2;
  earth.Forward(a.lat_deg, a.lon_deg, height_a.meters(), x1, y1, z1);
  earth.Forward(b.lat_deg, b.lon_deg, height_b.meters(), x2, y2, z2);
  const double dx = x1 - x2, dy = y1 - y2, dz = z1 - z2;
  return Distance::from_meters(std::sqrt(dx * dx + dy * dy + dz * dz));
}

Distance slant_from_ground(Distance ground, Distance height_a, Distance height_b) {
  // Law of cosines in the triangle earth-centre / antenna / target, written in
  // the half-angle (haversine) form s^2 = dh^2 + 4*r1*r2*sin^2(theta/2) to avoid
  // catastrophic cancellation between the large r1^2, r2^2 terms.
  const double theta = ground.meters() / kMeanR;  // central angle
  const double r1 = kMeanR + height_a.meters();
  const double r2 = kMeanR + height_b.meters();
  const double dh = height_a.meters() - height_b.meters();
  const double sh = std::sin(theta / 2.0);
  const double s2 = dh * dh + 4.0 * r1 * r2 * sh * sh;
  return Distance::from_meters(std::sqrt(std::max(0.0, s2)));
}

Distance ground_from_slant(Distance slant, Distance height_a, Distance height_b) {
  const double r1 = kMeanR + height_a.meters();
  const double r2 = kMeanR + height_b.meters();
  const double dh = height_a.meters() - height_b.meters();
  double sh2 = (slant.meters() * slant.meters() - dh * dh) / (4.0 * r1 * r2);
  sh2 = std::min(1.0, std::max(0.0, sh2));  // guard asin domain
  const double theta = 2.0 * std::asin(std::sqrt(sh2));
  return Distance::from_meters(kMeanR * theta);
}

Distance radar_horizon(Distance height, bool four_thirds) {
  const double h = std::max(0.0, height.meters());
  return Distance::from_meters(std::sqrt(2.0 * effective_radius(four_thirds) * h));
}

Distance radar_horizon_range(Distance height_a, Distance height_b, bool four_thirds) {
  return Distance::from_meters(radar_horizon(height_a, four_thirds).meters() +
                               radar_horizon(height_b, four_thirds).meters());
}

bool has_line_of_sight(LatLon a, Distance height_a, LatLon b, Distance height_b,
                       bool four_thirds) {
  const double ground = inverse_ellipsoidal(a, b).distance.meters();
  return ground <= radar_horizon_range(height_a, height_b, four_thirds).meters();
}

}  // namespace geo
