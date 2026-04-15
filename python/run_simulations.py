#!/usr/bin/env python3
"""
run_simulations.py
------------------
Corre Film_graphene variando el espesor del grafeno desde 5 hasta 60 um
en pasos de 5 um. Cada simulación lanza 100 000 neutrones y guarda el
archivo ROOT como build/output_<espesor>um.root
"""

import os
import sys
import subprocess
import shutil
import tempfile
from pathlib import Path

# ── Rutas ────────────────────────────────────────────────────────────────────
PROJECT_DIR = Path(__file__).resolve().parent.parent
BUILD_DIR   = PROJECT_DIR / "build"
EXECUTABLE  = BUILD_DIR / "Film_graphene"
MACROS_DIR  = PROJECT_DIR / "macros"

# Parámetros de barrido
THICKNESSES_UM = list(range(5, 65, 5))   # 5, 10, 15, …, 60
N_NEUTRONS     = 100000
KAPTON_UM      = 129   # espesor de kapton fijo

# ── Verificaciones previas ────────────────────────────────────────────────────
def check_prerequisites():
    if not EXECUTABLE.exists():
        sys.exit(f"[ERROR] No se encontró el ejecutable: {EXECUTABLE}\n"
                 "        Asegúrate de haber compilado el proyecto con cmake/make.")
    BUILD_DIR.mkdir(parents=True, exist_ok=True)


# ── Generar macro temporal ────────────────────────────────────────────────────
def make_macro(thickness_um: int) -> str:
    content = f"""/control/verbose 0
/run/verbose 0
/event/verbose 0
/tracking/verbose 0

/detector/grapheneThickness {thickness_um} um
/detector/kaptonThickness   {KAPTON_UM} um

/gun/particle neutron
/gun/energy 0.025 eV
/gun/position 0 0 -1.5 cm
/gun/direction 0 0 1
/gun/number 1

/run/beamOn {N_NEUTRONS}
"""
    return content


# ── Ejecutar una simulación ───────────────────────────────────────────────────
def run_simulation(thickness_um: int) -> bool:
    output_root = BUILD_DIR / f"output_{thickness_um}um.root"

    if output_root.exists():
        print(f"  [SKIP] {output_root.name} ya existe; borrándolo para regenerar.")
        output_root.unlink()

    # Crear macro temporal en el directorio build (el ejecutable lo busca desde su CWD)
    mac_path = BUILD_DIR / f"_tmp_{thickness_um}um.mac"
    mac_path.write_text(make_macro(thickness_um))

    print(f"\n{'='*60}")
    print(f"  Espesor grafeno: {thickness_um} um  —  {N_NEUTRONS} neutrones")
    print(f"{'='*60}")

    try:
        result = subprocess.run(
            [str(EXECUTABLE), str(mac_path)],
            cwd=str(BUILD_DIR),          # el ROOT se guarda donde corre el proceso
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
        )

        # Mostrar resumen del output (últimas 15 líneas)
        lines = result.stdout.strip().splitlines()
        summary_lines = [l for l in lines if any(
            kw in l for kw in ["fired", "Capture", "efficiency", "ROOT", "STATISTICS"]
        )]
        for l in (summary_lines if summary_lines else lines[-15:]):
            print(" ", l)

        if result.returncode != 0:
            print(f"  [WARN] El proceso terminó con código {result.returncode}")
            return False

        # Renombrar output.root → output_<X>um.root
        default_root = BUILD_DIR / "output.root"
        if default_root.exists():
            shutil.move(str(default_root), str(output_root))
            print(f"  [OK]  Guardado: {output_root.name}")
        else:
            print(f"  [ERROR] output.root no fue generado por la simulación.")
            return False

    except Exception as e:
        print(f"  [ERROR] Excepción al correr la simulación: {e}")
        return False

    finally:
        mac_path.unlink(missing_ok=True)

    return True


# ── Main ──────────────────────────────────────────────────────────────────────
def main():
    check_prerequisites()

    print(f"\nProyecto : {PROJECT_DIR}")
    print(f"Build    : {BUILD_DIR}")
    print(f"Espesores: {THICKNESSES_UM} um")
    print(f"Neutrones: {N_NEUTRONS} por simulación\n")

    results = {}
    for t in THICKNESSES_UM:
        ok = run_simulation(t)
        results[t] = "OK" if ok else "FAILED"

    # ── Resumen final ─────────────────────────────────────────────────────────
    print(f"\n{'='*60}")
    print("  RESUMEN FINAL")
    print(f"{'='*60}")
    for t, status in results.items():
        root_file = BUILD_DIR / f"output_{t}um.root"
        size_kb = root_file.stat().st_size // 1024 if root_file.exists() else 0
        print(f"  {t:3d} um  →  {status}  ({size_kb} KB)")

    n_ok = sum(1 for v in results.values() if v == "OK")
    print(f"\n  {n_ok}/{len(THICKNESSES_UM)} simulaciones completadas exitosamente.")


if __name__ == "__main__":
    main()
