#pragma once

#include <memory>
#include <string>

#include "geo/types.hpp"

namespace geo {

// Magnetic field elements at a point/time from the World Magnetic Model.
struct MagneticElements {
  double declination_deg;  // D, angle from true to magnetic north, + east
  double inclination_deg;  // I, dip angle, + down
  double horizontal_nT;    // H, horizontal intensity
  double total_nT;         // F, total intensity
};

// Wraps GeographicLib::MagneticModel (WMM2025 by default). `data_dir` must
// contain `<name>.wmm` and `<name>.wmm.cof`; the constructor throws
// (std::exception) if the model cannot be loaded or the time is out of range.
//
// HLR-GEO-020 declination / field elements
// HLR-GEO-021 true <-> magnetic bearing conversion
class MagneticField {
 public:
  explicit MagneticField(const std::string& data_dir,
                         const std::string& name = "wmm2025");
  ~MagneticField();
  MagneticField(MagneticField&&) noexcept;
  MagneticField& operator=(MagneticField&&) noexcept;
  MagneticField(const MagneticField&) = delete;
  MagneticField& operator=(const MagneticField&) = delete;

  MagneticElements field(LatLon p, double decimal_year,
                         double height_m = 0.0) const;
  double declination(LatLon p, double decimal_year,
                     double height_m = 0.0) const;

  // Magnetic bearing = true bearing − declination; true = magnetic + declination.
  Bearing true_to_magnetic(Bearing true_brg, LatLon p, double decimal_year,
                           double height_m = 0.0) const;
  Bearing magnetic_to_true(Bearing mag_brg, LatLon p, double decimal_year,
                           double height_m = 0.0) const;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace geo
