#include "geo/format.hpp"

#include <gtest/gtest.h>

#include <string>

namespace {

using geo::CoordFormat;
using geo::LatLon;

// HLR-GEO-011: decimal degrees parse straight through.
TEST(Parse, DecimalDegrees) {
  const auto p = geo::parse("37.4602 126.4407");
  EXPECT_NEAR(p.lat_deg, 37.4602, 1e-9);
  EXPECT_NEAR(p.lon_deg, 126.4407, 1e-9);
}

// HLR-GEO-011: DMS with hemisphere letters, including a western (negative) lon.
TEST(Parse, DmsWithHemispheres) {
  const auto p = geo::parse("40 26 46N 79 58 56W");
  EXPECT_NEAR(p.lat_deg, 40.446111, 1e-5);
  EXPECT_NEAR(p.lon_deg, -79.982222, 1e-5);
}

// HLR-GEO-010/011: every textual format round-trips back to the same position.
TEST(Format, RoundTripsThroughEveryFormat) {
  const LatLon p{37.4602, 126.4407};

  for (const std::string& s :
       {geo::to_dd(p), geo::to_dms(p), geo::to_ddm(p), geo::to_mgrs(p),
        geo::to_utm(p)}) {
    const auto back = geo::parse(s);
    EXPECT_NEAR(back.lat_deg, p.lat_deg, 2e-5) << "format: " << s;
    EXPECT_NEAR(back.lon_deg, p.lon_deg, 2e-5) << "format: " << s;
  }
}

// HLR-GEO-010: DD formatting shows the expected digits.
TEST(Format, DecimalDegreesText) {
  const auto s = geo::to_dd(LatLon{37.4602, 126.4407}, 4);
  EXPECT_NE(s.find("37.4602"), std::string::npos);
  EXPECT_NE(s.find("126.4407"), std::string::npos);
}

// HLR-GEO-010: MGRS precision is digits-per-coordinate (5 => 1 m), so the
// 1 m string carries 5 digits of easting + 5 of northing, and 100 m carries 3.
TEST(Format, MgrsPrecisionIsDigitsPerCoordinate) {
  const LatLon p{37.4602, 126.4407};
  EXPECT_EQ(geo::to_mgrs(p, 5), "52SBG7364349001");  // 1 m
  EXPECT_EQ(geo::to_mgrs(p, 3), "52SBG736490");       // 100 m
}

// HLR-GEO-012: format detection on representative inputs.
TEST(Detect, ClassifiesByShape) {
  EXPECT_EQ(geo::detect_format("37.4602 126.4407"), CoordFormat::DD);
  EXPECT_EQ(geo::detect_format("37 27.61N 126 26.44E"), CoordFormat::DDM);
  EXPECT_EQ(geo::detect_format("37 27 36N 126 26 26E"), CoordFormat::DMS);
  EXPECT_EQ(geo::detect_format("52SCH 12345 67890"), CoordFormat::MGRS);
  EXPECT_EQ(geo::detect_format("52n 323456 4567890"), CoordFormat::UTM);
  EXPECT_EQ(geo::detect_format("not a coordinate"), CoordFormat::Unknown);
}

// HLR-GEO-011: MGRS with grouping spaces ("52S CH 12345 67890") parses the same
// as the compact form (GeoCoords only accepts the compact spelling).
TEST(Parse, MgrsWithSpaces) {
  const auto spaced = geo::parse("52S CH 12345 67890");
  const auto compact = geo::parse("52SCH1234567890");
  EXPECT_NEAR(spaced.lat_deg, compact.lat_deg, 1e-9);
  EXPECT_NEAR(spaced.lon_deg, compact.lon_deg, 1e-9);
}

// HLR-GEO-011: bad input is rejected, not silently mis-parsed.
TEST(Parse, RejectsGarbage) {
  EXPECT_THROW(geo::parse("definitely not coords"), std::invalid_argument);
}

}  // namespace
