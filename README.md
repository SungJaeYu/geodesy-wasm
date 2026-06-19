# geodesy-wasm

Tactical coordinate / geodesy engine for fighter T&E and radar-tracking work.
Core geodesy math is **C++17 (wrapping GeographicLib) compiled to WebAssembly**;
the UI is a thin layer that calls the WASM module.

The signature operation is the forward geodesic problem — *lat/lon + true
bearing + distance → new coordinate* — extended to inverse, format conversion,
tactical coordinate systems (bullseye / BRAA), radar geometry (slant range,
horizon / line-of-sight), dead reckoning, and route/area calculations.

**▶ Live demo: https://sungjae-blog.vercel.app/geo/**

## Tabs (UI)

The browser tool is a thin layer over the WASM engine, with a shared
DD / DDM / DMS structured latitude/longitude input:

| Tab | What it does |
|-----|--------------|
| **Geodesic** | forward / inverse (great-circle + rhumb), intermediate / midpoint, DD·DMS·DDM·MGRS·UTM output, true↔magnetic bearings |
| **Tactical** | bullseye (to/from, click-map reference), BRAA bearing/range + aspect |
| **Radar** | slant ↔ ground range, radar horizon & line-of-sight (4/3-earth, cyan/amber), dead reckoning |
| **Route** | waypoint chain, radial & range-range intersection, cross/along-track, sector containment, polygon area |
| **Util** | map measure (two clicks), batch convert, CSV/KML/GPX import-export, terrain elevation |

## Status

All phases (1–4) complete. The engine has **36 GoogleTest known-answer tests**
(traced to `HLR-GEO-001..055` in `REQUIREMENTS.md`); CI runs the native suite
plus an Emscripten artifact build.

## Build & test

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build -L geo --output-on-failure   # -L geo: our suite only
```

GeographicLib (v2.7) and GoogleTest are fetched automatically on first
configure (needs network).

## WASM

```sh
./build_wasm.sh        # needs an active emsdk; emits web/geo.js + web/geo.wasm
```

## Layout

| Path | What |
|------|------|
| `include/geo/`, `src/` | C++ engine (`namespace geo`) |
| `bindings/embind.cpp` | WASM seam (plain doubles / value-objects) |
| `tests/` | GoogleTest known-answer suite (labeled `geo`) |
| `REQUIREMENTS.md` | HLR matrix, traced to tests |
| `CLAUDE.md` | guidance for working in this repo |
| `data/wmm/` | WMM2025 coefficients (GeographicLib format), embedded into the WASM |
| `WMM2025COF/` | NCEI source `.COF` + official test values (verification oracle) |

## Data attribution

The World Magnetic Model 2025 coefficients are produced by NOAA's National
Centers for Environmental Information (NCEI) and the British Geological Survey,
and are released into the public domain (U.S. Government work) for free public
and commercial use. Source: NCEI — https://www.ncei.noaa.gov/products/world-magnetic-model

See `CLAUDE.md` for conventions (typed units, requirements-first traceability,
the ellipsoidal/spherical cross-check, and why `parse()` rolls its own DMS
tokeniser).
