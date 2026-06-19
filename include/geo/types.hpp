#pragma once

#include <cmath>

// Core value types for the geodesy engine.
//
// Design rule (mirrors flightdyn): units and reference systems are made
// explicit in the type system, never assumed. A bare `double` never crosses an
// API boundary as a distance or a bearing — callers must say which unit they
// mean by constructing the right type.
namespace geo {

// Geographic position in decimal degrees on whatever datum the caller is
// working in (WGS84 throughout Phase 1). Latitude is positive north, longitude
// positive east.
struct LatLon {
  double lat_deg;
  double lon_deg;
};

// A length, stored internally in metres. Construct from an explicit unit and
// read back in any unit; the unit can never be left ambiguous at a call site.
class Distance {
 public:
  static Distance from_meters(double m) { return Distance(m); }
  static Distance from_km(double km) { return Distance(km * 1000.0); }
  static Distance from_nm(double nm) { return Distance(nm * 1852.0); }
  static Distance from_feet(double ft) { return Distance(ft * 0.3048); }

  double meters() const { return m_; }
  double km() const { return m_ / 1000.0; }
  double nm() const { return m_ / 1852.0; }
  double feet() const { return m_ / 0.3048; }

 private:
  explicit Distance(double m) : m_(m) {}
  double m_;
};

// A compass bearing in degrees, normalised to [0, 360). Whether it is a true or
// magnetic bearing is the caller's responsibility to track (true throughout
// Phase 1; magnetic conversion arrives in Phase 2).
class Bearing {
 public:
  static Bearing from_degrees(double deg) {
    double d = std::fmod(deg, 360.0);
    if (d < 0.0) d += 360.0;
    return Bearing(d);
  }

  double degrees() const { return deg_; }
  double radians() const { return deg_ * M_PI / 180.0; }

 private:
  explicit Bearing(double deg) : deg_(deg) {}
  double deg_;
};

}  // namespace geo
