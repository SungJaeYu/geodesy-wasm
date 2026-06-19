cmake_minimum_required(VERSION 3.18)
project(geodesy_wasm LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

# Enable C++ exception catching for the whole WASM build (default emscripten
# disables it, turning any throw — e.g. parse() on bad input, or GeographicLib's
# own errors — into a hard module abort instead of a catchable JS exception).
# Must be set before GeographicLib is fetched so its TUs get it too.
if(EMSCRIPTEN)
  add_compile_options(-fexceptions)
  add_link_options(-fexceptions)
endif()

# --- GeographicLib (static link) ----------------------------------------------
# Not header-only: fetched and built as a static library so it links into both
# the native test binary and the WASM module. Docs/manpages/tools off for speed.
# Static everything: required so the engine links into one self-contained WASM
# module (no .so dependencies the emscripten linker can't resolve).
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(GEOGRAPHICLIB_LIB_TYPE STATIC CACHE STRING "" FORCE)
set(BUILD_DOCUMENTATION OFF CACHE BOOL "" FORCE)
set(BUILD_MANPAGES OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
# Keep GeographicLib's own 200+ test suite out of our ctest run.
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
  geographiclib
  GIT_REPOSITORY https://github.com/geographiclib/geographiclib.git
  GIT_TAG v2.7
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(geographiclib)

# --- core engine library ------------------------------------------------------
add_library(geo STATIC
  src/geodesic.cpp
  src/format.cpp
  src/magnetic.cpp
  src/tactical.cpp
)
target_include_directories(geo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(geo PUBLIC GeographicLib::GeographicLib)
target_compile_features(geo PUBLIC cxx_std_17)
if(NOT MSVC)
  target_compile_options(geo PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(EMSCRIPTEN)
  # --- WASM module via embind -------------------------------------------------
  # Built only under emcmake. Emits geo.js (ES6 module) + geo.wasm.
  add_executable(geo_wasm bindings/embind.cpp)
  target_link_libraries(geo_wasm PRIVATE geo)
  set_target_properties(geo_wasm PROPERTIES OUTPUT_NAME "geo")
  target_link_options(geo_wasm PRIVATE
    --bind
    -sMODULARIZE=1
    -sEXPORT_ES6=1
    -sENVIRONMENT=web
    -sALLOW_MEMORY_GROWTH=1
    -sEXPORT_NAME=createGeoModule
  )
  # WMM coefficient files are embedded once they exist (Phase 2). Guarded so the
  # WASM build works before the data directory is populated.
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/data/wmm)
    target_link_options(geo_wasm PRIVATE
      "--embed-file=${CMAKE_CURRENT_SOURCE_DIR}/data/wmm@/data/wmm")
  endif()
else()
  # --- native GoogleTest suite ------------------------------------------------
  option(GEO_BUILD_TESTS "Build GoogleTest suite" ON)
  if(GEO_BUILD_TESTS)
    FetchContent_Declare(
      googletest
      URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()
    add_executable(geo_tests
      tests/geodesic.test.cpp
      tests/format.test.cpp
      tests/magnetic.test.cpp
      tests/tactical.test.cpp
    )
    target_link_libraries(geo_tests PRIVATE geo GTest::gtest_main)
    target_compile_options(geo_tests PRIVATE -Wall -Wextra)
    # The magnetic test loads the WMM data from the source tree.
    target_compile_definitions(geo_tests PRIVATE
      GEO_WMM_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/wmm")

    include(GoogleTest)
    # GeographicLib registers its own 200+ ctest entries with no opt-out, so we
    # label ours and run with `ctest -L geo` to keep the suite ours-only.
    gtest_discover_tests(geo_tests PROPERTIES LABELS "geo")
  endif()
endif()
