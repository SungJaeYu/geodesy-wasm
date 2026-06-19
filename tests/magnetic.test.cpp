#include "geo/magnetic.hpp"

#include <gtest/gtest.h>

#ifndef GEO_WMM_DATA_DIR
#define GEO_WMM_DATA_DIR ""
#endif

namespace {

using geo::Bearing;
using geo::LatLon;

geo::MagneticField model() { return geo::MagneticField(GEO_WMM_DATA_DIR); }

// HLR-GEO-020: declination + inclination match the official NOAA WMM2025 test
// values (WMM2025_TestValues.txt). Rows: {year, alt_km, lat, lon, D, I}.
TEST(Magnetic, MatchesOfficialTestValues) {
  struct Row {
    double year, alt_km, lat, lon, decl, incl;
  };
  const Row rows[] = {
      {2025.0, 65, 43, 93, 0.50, 64.10},
      {2025.0, 18, 0, 21, 1.29, -26.06},
      {2025.5, 44, 33, -118, 11.10, 57.89},
      {2026.0, 69, 23, 63, 1.17, 35.92},
  };
  const auto m = model();
  for (const Row& r : rows) {
    const auto e = m.field({r.lat, r.lon}, r.year, r.alt_km * 1000.0);
    EXPECT_NEAR(e.declination_deg, r.decl, 0.02)
        << "decl @ " << r.lat << "," << r.lon << " " << r.year;
    EXPECT_NEAR(e.inclination_deg, r.incl, 0.02)
        << "incl @ " << r.lat << "," << r.lon << " " << r.year;
  }
}

// HLR-GEO-021: true<->magnetic round-trips, and magnetic = true − declination.
TEST(Magnetic, TrueMagneticConversion) {
  const auto m = model();
  const LatLon p{33.0, -118.0};  // declination ~ +11.10 east at 2025.5, sea level
  const double year = 2025.5;
  const double decl = m.declination(p, year);

  const auto mag = m.true_to_magnetic(Bearing::from_degrees(90.0), p, year);
  EXPECT_NEAR(mag.degrees(), 90.0 - decl, 1e-9);

  const auto back = m.magnetic_to_true(mag, p, year);
  EXPECT_NEAR(back.degrees(), 90.0, 1e-9);
}

}  // namespace
