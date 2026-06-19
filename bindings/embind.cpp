// embind seam between the strongly-typed C++ core and JavaScript.
//
// Mirrors flightdyn's convention: the WASM boundary speaks plain doubles and
// value-objects, never the strong types. Distances cross as metres and bearings
// as degrees-true; unit conversion (NM/km/ft, true<->mag) lives in the TS layer.
#include <emscripten/bind.h>

#include <string>

#include "geo/format.hpp"
#include "geo/geodesic.hpp"
#include "geo/magnetic.hpp"
#include "geo/tactical.hpp"

using namespace emscripten;

namespace {

// JS-facing tactical results (flattened from geo::PolarFix / geo::Aspect).
struct PolarResult {
  double bearing_deg;
  double range_m;
};
struct AspectResult {
  double angle_deg;
  std::string side;  // "L", "R", or ""
};

PolarResult to_bullseye(geo::LatLon bullseye, geo::LatLon point) {
  const auto f = geo::to_bullseye(bullseye, point);
  return {f.bearing.degrees(), f.range.meters()};
}

geo::LatLon from_bullseye(geo::LatLon bullseye, double bearing_deg, double range_m) {
  return geo::from_bullseye(bullseye, geo::Bearing::from_degrees(bearing_deg),
                            geo::Distance::from_meters(range_m));
}

PolarResult braa(geo::LatLon ownship, geo::LatLon target) {
  const auto f = geo::braa(ownship, target);
  return {f.bearing.degrees(), f.range.meters()};
}

AspectResult aspect_angle(geo::LatLon ownship, geo::LatLon target,
                          double heading_deg) {
  const auto a =
      geo::aspect_angle(ownship, target, geo::Bearing::from_degrees(heading_deg));
  return {a.angle_deg, a.side == '\0' ? "" : std::string(1, a.side)};
}

// WMM data is --embed-file'd at /data/wmm (see CMakeLists). Loaded lazily on
// first use; throws (catchable in JS) if the data is missing or time out of range.
geo::MagneticField& wmm() {
  static geo::MagneticField model("/data/wmm");
  return model;
}

geo::MagneticElements magnetic_field(geo::LatLon p, double year, double height_m) {
  return wmm().field(p, year, height_m);
}

double declination(geo::LatLon p, double year, double height_m) {
  return wmm().declination(p, year, height_m);
}

double true_to_magnetic(double bearing_deg, geo::LatLon p, double year,
                        double height_m) {
  return wmm()
      .true_to_magnetic(geo::Bearing::from_degrees(bearing_deg), p, year, height_m)
      .degrees();
}

double magnetic_to_true(double bearing_deg, geo::LatLon p, double year,
                        double height_m) {
  return wmm()
      .magnetic_to_true(geo::Bearing::from_degrees(bearing_deg), p, year, height_m)
      .degrees();
}

// JS-facing result of the inverse problem (flattened from geo::GeodesicArc).
struct ArcResult {
  double initial_bearing_deg;
  double final_bearing_deg;
  double distance_m;
};

struct RhumbResult {
  double bearing_deg;
  double distance_m;
};

geo::LatLon forward_ellipsoidal(geo::LatLon start, double bearing_deg,
                                double dist_m) {
  return geo::forward_ellipsoidal(start, geo::Bearing::from_degrees(bearing_deg),
                                  geo::Distance::from_meters(dist_m));
}

geo::LatLon forward_spherical(geo::LatLon start, double bearing_deg,
                              double dist_m) {
  return geo::forward_spherical(start, geo::Bearing::from_degrees(bearing_deg),
                                geo::Distance::from_meters(dist_m));
}

ArcResult inverse_ellipsoidal(geo::LatLon p1, geo::LatLon p2) {
  const auto a = geo::inverse_ellipsoidal(p1, p2);
  return {a.initial_bearing.degrees(), a.final_bearing.degrees(),
          a.distance.meters()};
}

ArcResult inverse_spherical(geo::LatLon p1, geo::LatLon p2) {
  const auto a = geo::inverse_spherical(p1, p2);
  return {a.initial_bearing.degrees(), a.final_bearing.degrees(),
          a.distance.meters()};
}

RhumbResult inverse_rhumb(geo::LatLon p1, geo::LatLon p2) {
  const auto a = geo::inverse_rhumb(p1, p2);
  return {a.bearing.degrees(), a.distance.meters()};
}

int detect_format(const std::string& s) {
  return static_cast<int>(geo::detect_format(s));
}

}  // namespace

EMSCRIPTEN_BINDINGS(geo_module) {
  value_object<geo::LatLon>("LatLon")
      .field("lat", &geo::LatLon::lat_deg)
      .field("lon", &geo::LatLon::lon_deg);

  value_object<ArcResult>("ArcResult")
      .field("initialBearingDeg", &ArcResult::initial_bearing_deg)
      .field("finalBearingDeg", &ArcResult::final_bearing_deg)
      .field("distanceM", &ArcResult::distance_m);

  value_object<RhumbResult>("RhumbResult")
      .field("bearingDeg", &RhumbResult::bearing_deg)
      .field("distanceM", &RhumbResult::distance_m);

  value_object<geo::MagneticElements>("MagneticElements")
      .field("declinationDeg", &geo::MagneticElements::declination_deg)
      .field("inclinationDeg", &geo::MagneticElements::inclination_deg)
      .field("horizontalNT", &geo::MagneticElements::horizontal_nT)
      .field("totalNT", &geo::MagneticElements::total_nT);

  value_object<PolarResult>("PolarResult")
      .field("bearingDeg", &PolarResult::bearing_deg)
      .field("rangeM", &PolarResult::range_m);

  value_object<AspectResult>("AspectResult")
      .field("angleDeg", &AspectResult::angle_deg)
      .field("side", &AspectResult::side);

  function("forwardEllipsoidal", &forward_ellipsoidal);
  function("forwardSpherical", &forward_spherical);
  function("inverseEllipsoidal", &inverse_ellipsoidal);
  function("inverseSpherical", &inverse_spherical);
  function("inverseRhumb", &inverse_rhumb);
  function("intermediate", &geo::intermediate);
  function("midpoint", &geo::midpoint);

  function("toDD", &geo::to_dd);
  function("toDMS", &geo::to_dms);
  function("toDDM", &geo::to_ddm);
  function("toMGRS", &geo::to_mgrs);
  function("toUTM", &geo::to_utm);
  function("parse", &geo::parse);
  function("detectFormat", &detect_format);

  function("magneticField", &magnetic_field);
  function("declination", &declination);
  function("trueToMagnetic", &true_to_magnetic);
  function("magneticToTrue", &magnetic_to_true);

  function("toBullseye", &to_bullseye);
  function("fromBullseye", &from_bullseye);
  function("braa", &braa);
  function("aspectAngle", &aspect_angle);
}
