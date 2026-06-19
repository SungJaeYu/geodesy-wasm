#include "geo/geometry.hpp"

#include <gtest/gtest.h>

#include <vector>

#include "geo/geodesic.hpp"

namespace {

using geo::Bearing;
using geo::Distance;
using geo::LatLon;

// HLR-GEO-053: a point inside the bearing+range sector is contained; one outside
// the bearing wedge is not. Points are built at a known bearing/range.
TEST(Geometry, SectorContainment) {
  const LatLon c{37.0, 127.0};
  const auto inside = geo::forward_ellipsoidal(c, Bearing::from_degrees(90.0),
                                               Distance::from_km(50.0));
  const auto outside = geo::forward_ellipsoidal(c, Bearing::from_degrees(200.0),
                                                Distance::from_km(50.0));

  const auto bmin = Bearing::from_degrees(45.0);
  const auto bmax = Bearing::from_degrees(135.0);
  const auto rmin = Distance::from_km(0.0);
  const auto rmax = Distance::from_km(100.0);

  EXPECT_TRUE(geo::sector_contains(c, inside, bmin, bmax, rmin, rmax));
  EXPECT_FALSE(geo::sector_contains(c, outside, bmin, bmax, rmin, rmax));
  // inside the wedge but beyond max range -> not contained
  const auto farPt = geo::forward_ellipsoidal(c, Bearing::from_degrees(90.0),
                                              Distance::from_km(150.0));
  EXPECT_FALSE(geo::sector_contains(c, farPt, bmin, bmax, rmin, rmax));
}

// HLR-GEO-054: a 1deg x 1deg cell at the equator is ~1.23e10 m^2.
TEST(Geometry, PolygonAreaOfUnitCell) {
  const std::vector<LatLon> sq{{0, 0}, {0, 1}, {1, 1}, {1, 0}};
  const auto m = geo::polygon_area(sq);
  EXPECT_NEAR(m.area_m2, 1.234e10, 1e8);
  EXPECT_GT(m.perimeter_m, 0.0);
}

// HLR-GEO-055: centroid of the unit cell is its centre; inside/outside tests.
TEST(Geometry, CentroidAndContainment) {
  const std::vector<LatLon> sq{{0, 0}, {0, 1}, {1, 1}, {1, 0}};

  const auto c = geo::polygon_centroid(sq);
  EXPECT_NEAR(c.lat_deg, 0.5, 1e-9);
  EXPECT_NEAR(c.lon_deg, 0.5, 1e-9);

  EXPECT_TRUE(geo::point_in_polygon(sq, LatLon{0.5, 0.5}));
  EXPECT_FALSE(geo::point_in_polygon(sq, LatLon{5.0, 5.0}));
}

}  // namespace
