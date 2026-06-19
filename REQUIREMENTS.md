# Requirements (HLR) — geodesy-wasm

DO-178C-style high-level requirements for the **calculation core**. Each HLR is
implemented by a named function and traced to a GoogleTest case. The geodesic
core is the safety-relevant path (track/coordinate math); UI is a non-safety
consumer and is not traced here.

Verifiability: every HLR has a known-answer test whose reference value is
derived independently of the implementation (closed-form equator/meridian
geometry, or a forward/inverse round-trip), so a test passing proves the
behavior, not the code's agreement with itself.

## Phase 1 — Core

| HLR | Requirement | Function | Test (`tests/`) |
|-----|-------------|----------|-----------------|
| HLR-GEO-001 | Forward geodesic on WGS84 ellipsoid: start + true bearing + distance → destination | `forward_ellipsoidal` | `ForwardGeodesic.EllipsoidalDueEastAlongEquator`, `Geodesic.ForwardInverseRoundTrip` |
| HLR-GEO-002 | Forward on a sphere of mean WGS84 radius (great circle), for comparison | `forward_spherical` | `ForwardGeodesic.SphericalDueNorthFromEquator`, `ForwardGeodesic.EllipsoidalAndSphericalDiffer` |
| HLR-GEO-003 | Inverse geodesic (ellipsoidal): two points → initial/final bearing + distance | `inverse_ellipsoidal` | `InverseGeodesic.EllipsoidalAlongEquator`, `Geodesic.ForwardInverseRoundTrip` |
| HLR-GEO-004 | Inverse great circle (spherical) | `inverse_spherical` | `InverseGeodesic.SphericalAlongMeridian` |
| HLR-GEO-005 | Inverse rhumb line: constant bearing + loxodromic distance | `inverse_rhumb` | `InverseGeodesic.RhumbAlongEquator` |
| HLR-GEO-006 | Intermediate point at fraction f along a geodesic | `intermediate` | `Geodesic.IntermediateAndMidpointOnEquator` |
| HLR-GEO-007 | Midpoint of a geodesic | `midpoint` | `Geodesic.IntermediateAndMidpointOnEquator` |
| HLR-GEO-010 | Format a position as DD / DMS / DDM / MGRS / UTM | `to_dd` `to_dms` `to_ddm` `to_mgrs` `to_utm` | `Format.RoundTripsThroughEveryFormat`, `Format.DecimalDegreesText` |
| HLR-GEO-011 | Auto-detect and parse a free-form coordinate string (DD/DMS/DDM/MGRS/UTM); reject garbage | `parse` | `Parse.DecimalDegrees`, `Parse.DmsWithHemispheres`, `Parse.RejectsGarbage`, `Format.RoundTripsThroughEveryFormat` |
| HLR-GEO-012 | Best-effort format classification for UI hinting | `detect_format` | `Detect.ClassifiesByShape` |

## Phase 2 — Tactical & Radar (in progress)

| HLR | Requirement | Function | Test (`tests/`) |
|-----|-------------|----------|-----------------|
| HLR-GEO-020 | Magnetic field elements (declination, inclination, H, F) from WMM2025 at a point/time/height | `MagneticField::field` / `::declination` | `Magnetic.MatchesOfficialTestValues` |
| HLR-GEO-021 | True ↔ magnetic bearing conversion (magnetic = true − declination) | `MagneticField::true_to_magnetic` / `::magnetic_to_true` | `Magnetic.TrueMagneticConversion` |

Oracle: HLR-GEO-020 is verified against NOAA's official `WMM2025_TestValues.txt`
(declination + inclination matched to 0.02°). WMM data (`data/wmm/wmm2025.wmm`
+ `.cof`, GeographicLib format, same coefficients as the supplied `WMM.COF`) is
`--embed-file`'d into the WASM at `/data/wmm`.

Remaining Phase 2 (not yet implemented): bullseye, BRAA, slant/ground range,
radar horizon / LOS, dead reckoning. Then Phase 3 route/area (waypoint chains,
intersections, cross/along-track, sector containment, polygon). Each lands with
its own HLR-GEO-0xx row and known-answer test.
