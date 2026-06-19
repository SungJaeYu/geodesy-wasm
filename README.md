# geodesy-wasm

Tactical coordinate / geodesy engine for fighter T&E and radar-tracking work.
Core geodesy math is **C++17 (wrapping GeographicLib) compiled to WebAssembly**;
the UI is a thin layer that calls the WASM module.

The signature operation is the forward geodesic problem — *lat/lon + true
bearing + distance → new coordinate* — extended to inverse, format conversion,
and (in later phases) tactical coordinate systems (bullseye / BRAA), radar
geometry (slant range, horizon / line-of-sight), dead reckoning, and route/area
calculations.

## Status

- **Phase 1 — Core (done, native-tested):** forward / inverse / intermediate /
  midpoint geodesics (ellipsoidal + spherical + rhumb) and DD/DMS/DDM/MGRS/UTM
  conversion with an auto-detecting parser. 14 known-answer tests pass.
- **Phases 2–4 (planned):** magnetic true↔mag, bullseye/BRAA, radar
  slant/horizon/LOS, dead reckoning, waypoint chains, intersections,
  cross/along-track, sector/polygon. See the project spec and `REQUIREMENTS.md`.

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

See `CLAUDE.md` for conventions (typed units, requirements-first traceability,
the ellipsoidal/spherical cross-check, and why `parse()` rolls its own DMS
tokeniser).
