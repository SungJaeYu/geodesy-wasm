#include "geo/magnetic.hpp"

#include <GeographicLib/MagneticModel.hpp>

namespace geo {

struct MagneticField::Impl {
  GeographicLib::MagneticModel model;
  Impl(const std::string& dir, const std::string& name) : model(name, dir) {}
};

MagneticField::MagneticField(const std::string& data_dir, const std::string& name)
    : impl_(std::make_unique<Impl>(data_dir, name)) {}
MagneticField::~MagneticField() = default;
MagneticField::MagneticField(MagneticField&&) noexcept = default;
MagneticField& MagneticField::operator=(MagneticField&&) noexcept = default;

MagneticElements MagneticField::field(LatLon p, double year, double h) const {
  double bx = 0.0, by = 0.0, bz = 0.0;
  impl_->model(year, p.lat_deg, p.lon_deg, h, bx, by, bz);
  double H = 0.0, F = 0.0, D = 0.0, I = 0.0;
  GeographicLib::MagneticModel::FieldComponents(bx, by, bz, H, F, D, I);
  return MagneticElements{D, I, H, F};
}

double MagneticField::declination(LatLon p, double year, double h) const {
  return field(p, year, h).declination_deg;
}

Bearing MagneticField::true_to_magnetic(Bearing b, LatLon p, double year,
                                        double h) const {
  return Bearing::from_degrees(b.degrees() - declination(p, year, h));
}

Bearing MagneticField::magnetic_to_true(Bearing b, LatLon p, double year,
                                        double h) const {
  return Bearing::from_degrees(b.degrees() + declination(p, year, h));
}

}  // namespace geo
