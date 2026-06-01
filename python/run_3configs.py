#!/usr/bin/env python3
"""
run_3configs.py
---------------
Corre Film_graphene en 3 configuraciones y guarda
transmitted_particles_<config>.txt en build/.

Configuraciones:
  1. kaptonFirst   — kapton → grafeno → detector
  2. grapheneFirst — grafeno → kapton → detector
  3. noGraphene    — solo kapton → detector
"""

import os
import sys
import subprocess
import shutil
from pathlib import Path

PROJECT_DIR = Path(__file__).resolve().parent.parent
BUILD_DIR   = PROJECT_DIR / "build"
EXECUTABLE  = BUILD_DIR / "Film_graphene"

N_NEUTRONS   = 100_000
KAPTON_UM    = 125
GRAPHENE_UM  = 5
BORON_FRAC   = 0.05

CONFIGS = [
    {
        "name":            "kaptonFirst",
        "label":           "Kapton → Grafeno → Detector",
        "order":           "kaptonFirst",
        "enable_graphene": True,
    },
    {
        "name":            "grapheneFirst",
        "label":           "Grafeno → Kapton → Detector",
        "order":           "grapheneFirst",
        "enable_graphene": True,
    },
    {
        "name":            "noGraphene",
        "label":           "Solo Kapton → Detector (sin grafeno)",
        "order":           "kaptonFirst",
        "enable_graphene": False,
    },
]


def make_macro(cfg: dict) -> str:
    g_flag = "true" if cfg["enable_graphene"] else "false"
    return f"""/control/verbose 0
/run/verbose 0
/event/verbose 0
/tracking/verbose 0

/detector/grapheneThickness {GRAPHENE_UM} um
/detector/kaptonThickness   {KAPTON_UM} um
/detector/boronFraction     {BORON_FRAC}
/detector/order             {cfg['order']}
/detector/enableGraphene    {g_flag}

/run/initialize

/run/beamOn {N_NEUTRONS}
"""


def run_config(cfg: dict) -> bool:
    name = cfg["name"]
    mac_path = BUILD_DIR / f"_tmp_{name}.mac"
    mac_path.write_text(make_macro(cfg))

    print(f"\n{'='*60}")
    print(f"  Config: {name}  —  {cfg['label']}")
    print(f"{'='*60}")

    result = subprocess.run(
        [str(EXECUTABLE), str(mac_path)],
        cwd=str(BUILD_DIR),
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
    )
    mac_path.unlink(missing_ok=True)

    lines = result.stdout.strip().splitlines()
    for ln in lines:
        if any(kw in ln for kw in ["STATISTICS", "fired", "Capture", "efficiency", "ROOT", "WWWW", "Exception"]):
            print(" ", ln)

    if result.returncode != 0:
        print(f"  [WARN] código de retorno: {result.returncode}")

    # Guardar transmitted_particles.txt con nombre por config
    src = BUILD_DIR / "transmitted_particles.txt"
    dst = BUILD_DIR / f"transmitted_{name}.txt"
    if src.exists():
        shutil.copy(src, dst)
        nlines = sum(1 for l in dst.read_text().splitlines() if not l.startswith("#"))
        print(f"  [OK]  {dst.name}  ({nlines} partículas)")
    else:
        print(f"  [ERROR] transmitted_particles.txt no fue generado")
        return False

    # Guardar ROOT con nombre por config
    root_src = BUILD_DIR / "output.root"
    root_dst = BUILD_DIR / f"output_{name}.root"
    if root_src.exists():
        shutil.move(str(root_src), str(root_dst))
        print(f"  [OK]  {root_dst.name}")

    return True


def main():
    if not EXECUTABLE.exists():
        sys.exit(f"[ERROR] No se encontró: {EXECUTABLE}\nCompila el proyecto primero.")

    print(f"\nProyecto : {PROJECT_DIR}")
    print(f"Build    : {BUILD_DIR}")
    print(f"Neutrones: {N_NEUTRONS} por config")
    print(f"Kapton   : {KAPTON_UM} um  |  Grafeno: {GRAPHENE_UM} um  |  Boro: {BORON_FRAC*100:.0f}%")

    results = {}
    for cfg in CONFIGS:
        ok = run_config(cfg)
        results[cfg["name"]] = "OK" if ok else "FAILED"

    print(f"\n{'='*60}")
    print("  RESUMEN")
    print(f"{'='*60}")
    for name, status in results.items():
        f = BUILD_DIR / f"transmitted_{name}.txt"
        n = sum(1 for l in f.read_text().splitlines() if not l.startswith("#")) if f.exists() else 0
        print(f"  {name:<16}  {status}  ({n} partículas transmitidas)")


if __name__ == "__main__":
    main()
