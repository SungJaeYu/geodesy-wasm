// Functional smoke test of the WASM bindings (run with node from repo root):
//   node web/smoke.mjs
import { readFileSync } from "node:fs";
import createGeoModule from "./geo.js";

const wasmBinary = readFileSync(new URL("./geo.wasm", import.meta.url));
const M = await createGeoModule({ wasmBinary });

let failures = 0;
const near = (a, b, tol, msg) => {
  if (Math.abs(a - b) > tol) {
    console.error(`FAIL ${msg}: ${a} vs ${b}`);
    failures++;
  } else {
    console.log(`ok   ${msg}`);
  }
};

// forward: due east along equator by a*(pi/180) -> +1 deg lon
const a = 6378137.0;
const d = M.forwardEllipsoidal({ lat: 0, lon: 0 }, 90, a * Math.PI / 180);
near(d.lat, 0, 1e-9, "forwardEllipsoidal lat");
near(d.lon, 1, 1e-6, "forwardEllipsoidal lon");

// inverse round-trip
const arc = M.inverseEllipsoidal({ lat: 0, lon: 0 }, { lat: 0, lon: 1 });
near(arc.distanceM, a * Math.PI / 180, 1e-3, "inverse distance");
near(arc.initialBearingDeg, 90, 1e-9, "inverse bearing");

// parse + format
const p = M.parse("40 26 46N 79 58 56W");
near(p.lat, 40.446111, 1e-5, "parse DMS lat");
near(p.lon, -79.982222, 1e-5, "parse DMS lon");
console.log("toMGRS:", M.toMGRS({ lat: 37.4602, lon: 126.4407 }, 5));

// detectFormat enum (DD=0,DMS=1,DDM=2,MGRS=3,UTM=4,Unknown=5)
near(M.detectFormat("37.46 126.44"), 0, 0, "detectFormat DD");

// magnetic: WMM data embedded at /data/wmm; official value D=0.50 @ 43N,93E,65km,2025.0
const f = M.magneticField({ lat: 43, lon: 93 }, 2025.0, 65000);
near(f.declinationDeg, 0.5, 0.02, "declination (official WMM2025)");
near(f.inclinationDeg, 64.1, 0.02, "inclination (official WMM2025)");
const magBrg = M.trueToMagnetic(90, { lat: 43, lon: 93 }, 2025.0, 65000);
near(magBrg, 90 - f.declinationDeg, 1e-9, "true->magnetic bearing");

// tactical: bullseye round-trip and BRAA
const bull = { lat: 36.2, lon: 127.9 };
const tgt = M.fromBullseye(bull, 215, 80 * 1852);
const fix = M.toBullseye(bull, tgt);
near(fix.bearingDeg, 215, 1e-6, "bullseye round-trip bearing");
near(fix.rangeM / 1852, 80, 1e-6, "bullseye round-trip range");
const asp = M.aspectAngle({ lat: 0, lon: 1 }, { lat: 0, lon: 0 }, 0);
near(asp.angleDeg, 90, 1e-6, "aspect beam angle");
if (asp.side !== "R") { console.error("FAIL aspect side", asp.side); failures++; }
else console.log("ok   aspect side R");

// radar: slant (vertical), horizon (~41km @ 100m, 4/3), LOS
near(M.slantRange({ lat: 37, lon: 127 }, 0, { lat: 37, lon: 127 }, 9144), 9144, 1e-3, "slant vertical");
near(M.radarHorizon(100, true) / 1000, 41.22, 0.1, "radar horizon 4/3");
const losNear = M.forwardEllipsoidal({ lat: 37, lon: 127 }, 90, 50000);
if (!M.hasLineOfSight({ lat: 37, lon: 127 }, 100, losNear, 100)) { console.error("FAIL LOS near"); failures++; }
else console.log("ok   LOS within horizon");

// dead reckoning: 250 m/s for 60 s => ~15 km along atan2(vE,vN)
const dr = M.deadReckon({ lat: 37, lon: 127 }, 200, 150, 60);
const drArc = M.inverseEllipsoidal({ lat: 37, lon: 127 }, dr);
near(drArc.distanceM, 15000, 1, "dead reckon distance");

// exceptions must be catchable, not abort the module
try {
  M.parse("definitely not coords");
  console.error("FAIL parse-garbage did not throw");
  failures++;
} catch (e) {
  console.log("ok   parse-garbage threw");
}

console.log(failures === 0 ? "\nSMOKE PASS" : `\nSMOKE FAIL (${failures})`);
process.exit(failures === 0 ? 0 : 1);
