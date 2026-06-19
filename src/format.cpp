#include "geo/format.hpp"

#include <GeographicLib/DMS.hpp>
#include <GeographicLib/GeoCoords.hpp>

#include <cctype>
#include <iomanip>
#include <cmath>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace geo {
namespace {

// Encode one axis as "D M S<hemi>" / "D M.m<hemi>" with space separators, which
// GeographicLib parses straight back.
std::string encode_axis(double angle, GeographicLib::DMS::component trailing,
                        int decimals, GeographicLib::DMS::flag ind) {
  return GeographicLib::DMS::Encode(angle, trailing,
                                    static_cast<unsigned>(decimals), ind, ' ');
}

std::string trim(const std::string& s) {
  const auto b = s.find_first_not_of(" \t\r\n");
  if (b == std::string::npos) return "";
  const auto e = s.find_last_not_of(" \t\r\n");
  return s.substr(b, e - b + 1);
}

// Combine 1..3 sexagesimal components (deg, min, sec) into decimal degrees,
// preserving the sign of the leading component.
double sexagesimal(const std::vector<double>& parts) {
  double mag = 0.0;
  double scale = 1.0;
  for (double p : parts) {
    mag += std::abs(p) / scale;
    scale *= 60.0;
  }
  return (!parts.empty() && parts.front() < 0.0) ? -mag : mag;
}

// Parse a lat/lon string in any DD/DDM/DMS spelling, with any separators and
// optional N/S/E/W hemisphere letters. GeographicLib's GeoCoords can't read
// space-separated DMS, so we tokenise into number groups (terminated by
// hemisphere letters) ourselves.
LatLon parse_latlon_dms(const std::string& t) {
  struct Axis {
    double value;
    char hemi;  // 'N' 'S' 'E' 'W' or '\0'
  };
  std::vector<Axis> axes;
  std::vector<double> group;

  static const std::regex kToken(R"([+-]?\d+(?:\.\d+)?|[NSEWnsew])");
  for (auto it = std::sregex_iterator(t.begin(), t.end(), kToken);
       it != std::sregex_iterator(); ++it) {
    const std::string tok = it->str();
    const char c = static_cast<char>(std::toupper(static_cast<unsigned char>(tok[0])));
    if (tok.size() == 1 && (c == 'N' || c == 'S' || c == 'E' || c == 'W')) {
      double v = sexagesimal(group);
      if (c == 'S' || c == 'W') v = -std::abs(v);
      axes.push_back({v, c});
      group.clear();
    } else {
      group.push_back(std::stod(tok));
    }
  }
  // Numbers with no trailing hemisphere letter: either separate signed axes
  // (decimal-degree pair) or a single trailing sexagesimal axis.
  if (!group.empty()) {
    if (axes.empty()) {
      for (double n : group) axes.push_back({n, '\0'});
    } else {
      axes.push_back({sexagesimal(group), '\0'});
    }
  }

  if (axes.size() != 2) {
    throw std::invalid_argument("expected two coordinates");
  }

  // Order by hemisphere when present, otherwise latitude first.
  double lat = axes[0].value;
  double lon = axes[1].value;
  if (axes[0].hemi != '\0' && axes[1].hemi != '\0') {
    for (const Axis& a : axes) {
      if (a.hemi == 'N' || a.hemi == 'S') lat = a.value;
      else lon = a.value;
    }
  }
  return LatLon{lat, lon};
}

}  // namespace

std::string to_dd(LatLon p, int decimals) {
  std::ostringstream os;
  os << std::fixed << std::setprecision(decimals) << p.lat_deg << ' '
     << p.lon_deg;
  return os.str();
}

std::string to_dms(LatLon p, int decimals) {
  return encode_axis(p.lat_deg, GeographicLib::DMS::SECOND, decimals,
                     GeographicLib::DMS::LATITUDE) +
         ' ' +
         encode_axis(p.lon_deg, GeographicLib::DMS::SECOND, decimals,
                     GeographicLib::DMS::LONGITUDE);
}

std::string to_ddm(LatLon p, int decimals) {
  return encode_axis(p.lat_deg, GeographicLib::DMS::MINUTE, decimals,
                     GeographicLib::DMS::LATITUDE) +
         ' ' +
         encode_axis(p.lon_deg, GeographicLib::DMS::MINUTE, decimals,
                     GeographicLib::DMS::LONGITUDE);
}

std::string to_mgrs(LatLon p, int precision) {
  GeographicLib::GeoCoords g(p.lat_deg, p.lon_deg);
  // Our `precision` is digits per coordinate (5 => 1 m, the standard MGRS
  // notion). GeographicLib counts relative to 1 m (prec 0 => 1 m, -5 => 100 km),
  // i.e. digits-per-coordinate minus 5.
  return g.MGRSRepresentation(precision - 5);
}

std::string to_utm(LatLon p) {
  GeographicLib::GeoCoords g(p.lat_deg, p.lon_deg);
  return g.UTMUPSRepresentation();
}

LatLon parse(const std::string& text) {
  const std::string t = trim(text);
  if (t.empty()) throw std::invalid_argument("empty coordinate");

  const CoordFormat fmt = detect_format(t);
  try {
    if (fmt == CoordFormat::MGRS || fmt == CoordFormat::UTM) {
      GeographicLib::GeoCoords g(t);
      return LatLon{g.Latitude(), g.Longitude()};
    }
    return parse_latlon_dms(t);
  } catch (const std::invalid_argument&) {
    throw;
  } catch (const std::exception& e) {
    throw std::invalid_argument(std::string("unparseable coordinate: ") +
                                e.what());
  }
}

CoordFormat detect_format(const std::string& text) {
  const std::string t = trim(text);
  if (t.empty()) return CoordFormat::Unknown;

  // Compact, upper-cased copy for grid-format matching.
  std::string compact;
  for (char c : t) {
    if (!std::isspace(static_cast<unsigned char>(c))) {
      compact.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
    }
  }

  static const std::regex kMgrs(R"(^\d{1,2}[C-X][A-Z]{2}\d+$)");
  if (std::regex_match(compact, kMgrs)) return CoordFormat::MGRS;

  static const std::regex kUtm(R"(^\d{1,2}\s*[C-X]?\s*[NS]?\s+\d+\s+\d+$)",
                               std::regex::icase);
  if (std::regex_match(t, kUtm)) return CoordFormat::UTM;

  // Otherwise it's lat/lon text — classify by how many numeric groups appear:
  // 2 => decimal degrees, 4 => deg + decimal minutes, 6 => deg/min/sec.
  static const std::regex kNum(R"(\d+(?:\.\d+)?)");
  const auto begin = std::sregex_iterator(t.begin(), t.end(), kNum);
  const auto count = std::distance(begin, std::sregex_iterator());
  if (count == 2) return CoordFormat::DD;
  if (count == 4) return CoordFormat::DDM;
  if (count >= 6) return CoordFormat::DMS;
  return CoordFormat::Unknown;
}

}  // namespace geo
