#!/usr/bin/env python3
"""
run_simulations_boron.py
------------------------
Corre Film_graphene variando la fracción másica de B-10 en el grafeno.
El espesor del grafeno se mantiene fijo (GRAPHENE_UM).

Fracciones de boro barridas: BORON_FRACTIONS
Cada simulación lanza N_NEUTRONS neutrones y guarda el archivo ROOT como
  build/output_B<X>pct.root   (X = fracción en porcentaje, e.g. B5pct = 5 %)
"""

import os
import sys
import subprocess
import shutil
from pathlib import Path

# ── Rutas ────────────────────────────────────────────────────────────────────
PROJECT_DIR = Path(__file__).resolve().parent.parent
BUILD_DIR   = PROJECT_DIR / "build"
EXECUTABLE  = BUILD_DIR / "Film_graphene"

# ── Parámetros de barrido ─────────────────────────────────────────────────────
# Fracción másica de B-10 (0–1). Se puede ajustar libremente.
BORON_FRACTIONS = [0.01, 0.02, 0.03, 0.05, 0.07, 0.10, 0.15, 0.20, 0.25, 0.30]

# Espesor de grafeno fijo (en µm) — usar el óptimo de la barrida en espesor
GRAPHENE_UM = 20
KAPTON_UM   = 129   # espesor de kapton (fijo)
N_NEUTRONS  = 100_000


# ── Verificaciones previas ────────────────────────────────────────────────────
def check_prerequisites():
    if not EXECUTABLE.exists():
        sys.exit(f"[ERROR] No se encontró el ejecutable: {EXECUTABLE}\n"
                 "        Asegúrate de haber compilado el proyecto con cmake/make.")
    BUILD_DIR.mkdir(parents=True, exist_ok=True)


# ── Nombre de archivo ROOT para una fracción dada ─────────────────────────────
def root_name(fraction: float) -> str:
    """Convierte 0.05 → 'output_B5.00pct.root' (dos decimales)."""
    pct = fraction * 100
    return f"output_B{pct:.2f}pct.root"


# ── Generar macro temporal ────────────────────────────────────────────────────
def make_macro(fraction: float) -> str:
    return f"""/control/verbose 0
/run/verbose 0
/event/verbose 0
/tracking/verbose 0

/detector/grapheneThickness {GRAPHENE_UM} um
/detector/kaptonThickness   {KAPTON_UM} um
/detector/boronFraction     {fraction}

/gun/particle neutron
/gun/energy 0.025 eV
/gun/position 0 0 -1.5 cm
/gun/direction 0 0 1
/gun/number 1

/run/beamOn {N_NEUTRONS}
"""


# ── Ejecutar una simulación ───────────────────────────────────────────────────
def run_simulation(fraction: float) -> bool:
    output_root = BUILD_DIR / root_name(fraction)

    if output_root.exists():
        print(f"  [SKIP] {output_root.name} ya existe; borrándolo para regenerar.")
        output_root.unlink()

    mac_path = BUILD_DIR / f"_tmp_B{fraction:.4f}.mac"
    mac_path.write_text(make_macro(fraction))

    pct = fraction * 100
    print(f"\n{'='*60}")
    print(f"  Boro: {pct:.2f} %  |  Grafeno: {GRAPHENE_UM} µm  |  {N_NEUTRONS} neutrones")
    print(f"{'='*60}")

    try:
        result = subprocess.run(
            [str(EXECUTABLE), str(mac_path)],
            cwd=str(BUILD_DIR),
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
        )

        lines = result.stdout.strip().splitlines()
        summary_lines = [l for l in lines if any(
            kw in l for kw in ["fired", "Capture", "efficiency", "ROOT", "STATISTICS", "Geometria"]
        )]
        for l in (summary_lines if summary_lines else lines[-15:]):
            print(" ", l)

        if result.returncode != 0:
            print(f"  [WARN] El proceso terminó con código {result.returncode}")
            return False

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

    print(f"\nProyecto  : {PROJECT_DIR}")
    print(f"Build     : {BUILD_DIR}")
    print(f"Grafeno   : {GRAPHENE_UM} µm (fijo)")
    print(f"Boro      : {[f'{f*100:.2f}%' for f in BORON_FRACTIONS]}")
    print(f"Neutrones : {N_NEUTRONS} por simulación\n")

    results = {}
    for f in BORON_FRACTIONS:
        ok = run_simulation(f)
        results[f] = "OK" if ok else "FAILED"

    # ── Resumen final ──────────────────────────────────────────────────────────
    print(f"\n{'='*60}")
    print("  RESUMEN FINAL")
    print(f"{'='*60}")
    for f, status in results.items():
        rfile = BUILD_DIR / root_name(f)
        size_kb = rfile.stat().st_size // 1024 if rfile.exists() else 0
        print(f"  B {f*100:5.2f} %  →  {status}  ({size_kb} KB)  [{root_name(f)}]")

    n_ok = sum(1 for v in results.values() if v == "OK")
    print(f"\n  {n_ok}/{len(BORON_FRACTIONS)} simulaciones completadas exitosamente.")


if __name__ == "__main__":
    main()
