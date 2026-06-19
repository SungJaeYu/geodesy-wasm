#pragma once

#include <string>

#include "geo/types.hpp"

namespace geo {

// Coordinate text formats the tool understands.
//   DD   decimal degrees            "37.4602 126.4407"
//   DMS  degrees/minutes/seconds    "37 27 36.7N 126 26 26.5E"
//   DDM  degrees/decimal minutes    "37 27.61N 126 26.44E"
//   MGRS military grid              "52SCH 12345 67890"
//   UTM  universal transverse merc. "52n 323456 4567890"
enum class CoordFormat { DD, DMS, DDM, MGRS, UTM, Unknown };

// --- Formatting: a position to a string ---------------------------------------
// HLR-GEO-010
std::string to_dd(LatLon p, int decimals = 6);
std::string to_dms(LatLon p, int decimals = 2);
std::string to_ddm(LatLon p, int decimals = 4);
std::string to_mgrs(LatLon p, int precision = 5);  // 5 digits => 1 m
std::string to_utm(LatLon p);

// --- Parsing: a free-form string to a position --------------------------------
// `parse` auto-detects DD/DMS/DDM/MGRS/UTM (via GeographicLib::GeoCoords) and
// returns the position; it throws std::invalid_argument on unparseable input.
// `detect_format` is a best-effort classifier for UI hinting only — parsing is
// always authoritative.
// HLR-GEO-011 parse   HLR-GEO-012 detect_format
LatLon parse(const std::string& text);
CoordFormat detect_format(const std::string& text);

}  // namespace geo
