# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

A tactical coordinate / geodesy tool for fighter T&E and radar tracking work.
The **calculation engine is C++17 compiled to WASM via Emscripten**; the UI is a
thin layer that calls the WASM module. Keep that split — geodesy math lives in
C++ (`namespace geo`), never in the UI.

The engine wraps **GeographicLib** (fetched and statically linked, not
header-only) and adds a tactical layer on top. The same library covers geodesic
+ MGRS/UTM + magnetic + ECEF/ENU + intersection + polygon, so prefer it over
hand-rolled math — except that each GeographicLib-backed result has a
hand-rolled **spherical** counterpart kept alongside for comparison/validation
(e.g. `forward_ellipsoidal` vs `forward_spherical`).

Phase 1 (forward/inverse/intermediate/midpoint + format conversion + parser) is
implemented and tested natively. Phases 2–4 (magnetic, bullseye/BRAA, radar
geometry, dead reckoning, routes/areas) are specified but not yet built.

## Commands

```sh
# Native build + test (GeographicLib + GoogleTest fetched on first configure)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j

# Run OUR test suite only. GeographicLib registers ~200 of its own ctest
# entries with no opt-out, so our tests carry the `geo` label — always pass -L.
ctest --test-dir build -L geo --output-on-failure

# A single test
ctest --test-dir build -R Format.RoundTripsThroughEveryFormat --output-on-failure
./build/geo_tests --gtest_filter='InverseGeodesic.*'

# WASM build -> web/geo.js + web/geo.wasm (needs emsdk active in the shell)
./build_wasm.sh
```

`cmake` is available and **emsdk is installed at `~/emsdk`** — activate it with
`source ~/emsdk/emsdk_env.sh` before `build_wasm.sh`. CI
(`.github/workflows/ci.yml`) runs native ctest on one job and an Emscripten
artifact build on another.

## Architecture

```
include/geo/   public headers (one per concern)
src/           implementations (GeographicLib wrappers + spherical fallbacks)
bindings/      embind.cpp — the WASM seam
tests/         GoogleTest known-answer tests, labeled `geo`
web/           build_wasm.sh output (geo.js/geo.wasm) + web/smoke.mjs node check
data/wmm/      WMM coefficients, --embed-file'd into WASM (Phase 2)
```

**The UI lives in a separate repo**, not here: the Astro blog at
`../SungjaeYuBlog/astro-paper`. The tool is a self-contained static app at
`public/geo/` there (`index.html` + `app.js` + copied `geo.js`/`geo.wasm`),
mirroring how `public/flightdyn/` is hosted, and is registered as a project in
that repo's `src/data/projects.ts` (`embed: "/geo/"`). It uses Leaflet (CDN) with
CARTO dark tiles and the glass-cockpit design tokens. Deploy after a WASM rebuild
by copying `web/geo.js`/`web/geo.wasm` into the blog's `public/geo/`. Verify it
in a browser with `node web/smoke.mjs` (module-level) or by serving `public/geo/`.

- **`types.hpp` is the vocabulary.** `LatLon` (degrees), `Distance` (stored in
  metres, constructed via `from_nm/_km/_meters/_feet`), `Bearing` (normalised
  to [0,360)). Units and reference systems are explicit in the type system —
  a bare `double` never crosses an engine API as a distance or bearing, and
  every function names its frame (true/mag, GC/rhumb, ellipsoid/sphere).
- **`bindings/embind.cpp` is the C++↔JS seam.** Like flightdyn's `solve()`, the
  WASM boundary speaks **plain doubles + value-objects**, never the strong
  types: distances cross as metres, bearings as degrees-true. Unit conversion
  (NM/km/ft) and true↔magnetic belong in the TS wrapper, not the binding.
- **Every GeographicLib result keeps a spherical sibling** so the two models can
  be diffed in tests and surfaced as a UI toggle. Don't delete the "redundant"
  spherical code — it's the cross-check.

## Conventions (don't break these)

- **Requirements-first / traceability.** Every engine behavior is an `HLR-GEO-0xx`
  in `REQUIREMENTS.md`, implemented by a named function, traced to a test tagged
  with its HLR id (see `// HLR-...` comments in tests). Reference values are
  closed-form (equator/meridian geometry) or round-trips, chosen so a passing
  test proves the behavior independently of the implementation. Update the
  matrix when adding behavior.
- **TDD.** New function ⇒ failing known-answer test first. The spherical/
  ellipsoidal split exists partly to make answers checkable.
- **`parse()` does its own DMS tokenising.** GeographicLib's `GeoCoords` cannot
  read space-separated DMS (`37 26 30N`), which users paste, so `parse` handles
  DD/DDM/DMS itself (any separators, optional N/S/E/W) and only delegates to
  `GeoCoords` for MGRS/UTM. `detect_format` is a heuristic for UI hinting only —
  `parse` is authoritative.
- **GeographicLib is static + warnings-isolated.** Our targets compile with
  `-Wall -Wextra` (`-Wpedantic` on the library target); keep them clean.

## Build/toolchain notes

- GeographicLib is pinned to `v2.7` via `FetchContent` (`GIT_SHALLOW`), built
  `STATIC` with docs/manpages/`BUILD_TESTING` off.
- The WASM target (`geo_wasm` → `geo.js`/`geo.wasm`) is only added under
  `emcmake` (`if(EMSCRIPTEN)`); native configure builds the library + tests.
- WMM data is `--embed-file`'d only when `data/wmm/` exists, so the WASM build
  works before Phase 2 populates it.
