#!/usr/bin/env python3
"""
compare_transmitted.py
-----------------------
Lee los 3 transmitted_particles_<config>.txt y genera
análisis comparativo: tipos de partícula, espectros de energía,
fracciones de transmisión y origen de las partículas.
"""

import sys
from pathlib import Path
import numpy as np
import pandas as pd
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec

BUILD_DIR  = Path(__file__).resolve().parent.parent / "build"
OUTPUT_DIR = Path(__file__).resolve().parent / "plots_3configs"
OUTPUT_DIR.mkdir(exist_ok=True)

N_PRIMARIES = 100_000

CONFIGS = {
    "kaptonFirst":   "Kapton → Grafeno",
    "grapheneFirst": "Grafeno → Kapton",
    "noGraphene":    "Solo Kapton",
}

COLORS = {
    "kaptonFirst":   "#2196F3",   # azul
    "grapheneFirst": "#4CAF50",   # verde
    "noGraphene":    "#FF5722",   # naranja
}

# Columnas del archivo ASCII (ver TransmittedSD.cc)
COLS = ["EventID", "Particle", "KinEnergy_MeV",
        "DirX", "DirY", "DirZ",
        "VertexVolume", "CreatorProcess", "TargetZ", "TargetA"]


# ── Carga ──────────────────────────────────────────────────────────────────────
def load(name: str) -> pd.DataFrame:
    path = BUILD_DIR / f"transmitted_{name}.txt"
    if not path.exists():
        print(f"[WARN] No se encontró {path.name} — saltando.")
        return pd.DataFrame(columns=COLS)
    df = pd.read_csv(path, sep=r"\s+", comment="#", names=COLS)
    return df


def load_all() -> dict[str, pd.DataFrame]:
    dfs = {}
    for name in CONFIGS:
        dfs[name] = load(name)
        n = len(dfs[name])
        print(f"  {name:<16}: {n:>7,} partículas transmitidas  ({100*n/N_PRIMARIES:.2f}%)")
    return dfs


# ── Plot 1: conteo de partículas por tipo ─────────────────────────────────────
def plot_particle_counts(dfs):
    all_particles = sorted(set(p for df in dfs.values() for p in df["Particle"].unique()))
    x = np.arange(len(all_particles))
    width = 0.25

    fig, ax = plt.subplots(figsize=(12, 5))
    for i, (name, label) in enumerate(CONFIGS.items()):
        df = dfs[name]
        counts = [len(df[df["Particle"] == p]) for p in all_particles]
        bars = ax.bar(x + i*width, counts, width, label=label, color=COLORS[name], alpha=0.85)

    ax.set_xticks(x + width)
    ax.set_xticklabels(all_particles, rotation=30, ha="right", fontsize=9)
    ax.set_ylabel("Partículas transmitidas")
    ax.set_title("Conteo por tipo de partícula — 3 configuraciones")
    ax.legend()
    ax.set_yscale("log")
    ax.grid(axis="y", alpha=0.3)
    fig.tight_layout()
    fig.savefig(OUTPUT_DIR / "01_particle_counts.png", dpi=150)
    plt.close(fig)
    print("  Guardado: 01_particle_counts.png")


# ── Plot 2: fracción de transmisión por tipo ───────────────────────────────────
def plot_transmission_fraction(dfs):
    all_particles = sorted(set(p for df in dfs.values() for p in df["Particle"].unique()))
    x = np.arange(len(all_particles))
    width = 0.25

    fig, ax = plt.subplots(figsize=(12, 5))
    for i, (name, label) in enumerate(CONFIGS.items()):
        df = dfs[name]
        fracs = [100 * len(df[df["Particle"] == p]) / N_PRIMARIES for p in all_particles]
        ax.bar(x + i*width, fracs, width, label=label, color=COLORS[name], alpha=0.85)

    ax.set_xticks(x + width)
    ax.set_xticklabels(all_particles, rotation=30, ha="right", fontsize=9)
    ax.set_ylabel("Transmisión (% de primarios)")
    ax.set_title("Fracción de transmisión por tipo de partícula")
    ax.legend()
    ax.grid(axis="y", alpha=0.3)
    fig.tight_layout()
    fig.savefig(OUTPUT_DIR / "02_transmission_fraction.png", dpi=150)
    plt.close(fig)
    print("  Guardado: 02_transmission_fraction.png")


# ── Plot 3: espectro de energía de neutrones ───────────────────────────────────
def plot_neutron_spectra(dfs):
    fig, axes = plt.subplots(1, 3, figsize=(15, 4), sharey=True)
    bins = np.logspace(-5, 2, 80)

    for ax, (name, label) in zip(axes, CONFIGS.items()):
        df = dfs[name]
        n = df[df["Particle"] == "neutron"]
        if len(n):
            ax.hist(n["KinEnergy_MeV"] * 1e6, bins=bins,
                    color=COLORS[name], alpha=0.8, edgecolor="none")
        ax.set_xscale("log")
        ax.set_xlabel("Energía (eV)")
        ax.set_title(label, fontsize=10)
        ax.axvline(0.025, color="red", ls="--", lw=1, label="0.025 eV")
        ax.axvline(0.5,   color="orange", ls="--", lw=1, label="0.5 eV")
        ax.legend(fontsize=7)
        ax.grid(alpha=0.3)

    axes[0].set_ylabel("Cuentas")
    fig.suptitle("Espectro de energía — Neutrones transmitidos", fontsize=12)
    fig.tight_layout()
    fig.savefig(OUTPUT_DIR / "03_neutron_spectra.png", dpi=150)
    plt.close(fig)
    print("  Guardado: 03_neutron_spectra.png")


# ── Plot 4: espectro de gammas (todos los configs superpuestos) ─────────────────
def plot_gamma_spectra(dfs):
    fig, ax = plt.subplots(figsize=(10, 5))
    bins = np.logspace(-3, 1, 80)

    for name, label in CONFIGS.items():
        df = dfs[name]
        g = df[df["Particle"] == "gamma"]
        if len(g):
            ax.hist(g["KinEnergy_MeV"] * 1000, bins=bins * 1000,
                    histtype="step", lw=2, label=f"{label} (n={len(g):,})",
                    color=COLORS[name])

    ax.set_xscale("log")
    ax.set_xlabel("Energía (keV)")
    ax.set_ylabel("Cuentas")
    ax.set_title("Espectro de energía — Gammas transmitidos")
    ax.axvline(478, color="purple", ls="--", lw=1.5, label="478 keV (captura B-10)")
    ax.legend()
    ax.grid(alpha=0.3)
    fig.tight_layout()
    fig.savefig(OUTPUT_DIR / "04_gamma_spectra.png", dpi=150)
    plt.close(fig)
    print("  Guardado: 04_gamma_spectra.png")


# ── Plot 5: origen de las partículas (VertexVolume) ────────────────────────────
def plot_vertex_volume(dfs):
    fig, axes = plt.subplots(1, 3, figsize=(15, 4))

    for ax, (name, label) in zip(axes, CONFIGS.items()):
        df = dfs[name]
        if df.empty:
            ax.set_title(label)
            continue
        counts = df["VertexVolume"].value_counts()
        ax.pie(counts.values, labels=counts.index,
               autopct="%1.1f%%", colors=plt.cm.Set3.colors[:len(counts)])
        ax.set_title(label, fontsize=10)

    fig.suptitle("Origen de partículas transmitidas (VertexVolume)", fontsize=12)
    fig.tight_layout()
    fig.savefig(OUTPUT_DIR / "05_vertex_volume.png", dpi=150)
    plt.close(fig)
    print("  Guardado: 05_vertex_volume.png")


# ── Plot 6: superposición espectro neutrones ───────────────────────────────────
def plot_neutron_overlay(dfs):
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 5))
    bins_log = np.logspace(-5, 2, 100)
    bins_lin = np.linspace(0, 1e-4, 80)   # región térmica en eV (convertido a MeV: 1e-4/1e6)

    for name, label in CONFIGS.items():
        df = dfs[name]
        n = df[df["Particle"] == "neutron"]["KinEnergy_MeV"]
        if len(n) == 0:
            continue
        e_eV = n * 1e6
        ax1.hist(e_eV, bins=bins_log, histtype="step", lw=2,
                 label=f"{label} (n={len(n):,})", color=COLORS[name])
        # región térmica: < 1 eV
        thermal = e_eV[e_eV < 1.0]
        if len(thermal):
            ax2.hist(thermal, bins=np.logspace(-5, 0, 80),
                     histtype="step", lw=2, label=f"{label} (n={len(thermal):,})",
                     color=COLORS[name])

    for ax in (ax1, ax2):
        ax.set_xscale("log")
        ax.set_xlabel("Energía (eV)")
        ax.set_ylabel("Cuentas")
        ax.legend(fontsize=8)
        ax.grid(alpha=0.3)
    ax1.set_title("Espectro completo (10⁻⁵ — 10² eV)")
    ax2.set_title("Región térmica (< 1 eV)")
    ax1.axvline(0.025, color="red", ls="--", lw=1, label="0.025 eV")

    fig.suptitle("Comparación — Neutrones transmitidos", fontsize=12)
    fig.tight_layout()
    fig.savefig(OUTPUT_DIR / "06_neutron_overlay.png", dpi=150)
    plt.close(fig)
    print("  Guardado: 06_neutron_overlay.png")


# ── Tabla resumen ──────────────────────────────────────────────────────────────
def print_summary_table(dfs):
    print(f"\n{'='*70}")
    print(f"{'Config':<16} {'Total':>8} {'neutron':>8} {'gamma':>8} {'e-':>8} {'Li7':>8} {'alpha':>8}")
    print(f"{'-'*70}")
    for name, label in CONFIGS.items():
        df = dfs[name]
        tot = len(df)
        def cnt(p): return len(df[df["Particle"] == p])
        print(f"{name:<16} {tot:>8,} {cnt('neutron'):>8,} {cnt('gamma'):>8,}"
              f" {cnt('e-'):>8,} {cnt('Li7'):>8,} {cnt('alpha'):>8,}")
    print(f"{'='*70}")

    # CSV
    rows = []
    all_p = sorted(set(p for df in dfs.values() for p in df["Particle"].unique()))
    for name in CONFIGS:
        df = dfs[name]
        row = {"config": name, "total": len(df)}
        for p in all_p:
            row[p] = len(df[df["Particle"] == p])
        rows.append(row)
    pd.DataFrame(rows).to_csv(OUTPUT_DIR / "summary.csv", index=False)
    print(f"\n  CSV guardado: {OUTPUT_DIR / 'summary.csv'}")


# ── Main ───────────────────────────────────────────────────────────────────────
def main():
    print(f"\nLeyendo archivos desde: {BUILD_DIR}")
    dfs = load_all()

    if all(df.empty for df in dfs.values()):
        sys.exit("[ERROR] No se encontró ningún archivo — corre run_3configs.py primero.")

    print(f"\nGenerando gráficas en: {OUTPUT_DIR}")
    plot_particle_counts(dfs)
    plot_transmission_fraction(dfs)
    plot_neutron_spectra(dfs)
    plot_gamma_spectra(dfs)
    plot_vertex_volume(dfs)
    plot_neutron_overlay(dfs)
    print_summary_table(dfs)


if __name__ == "__main__":
    main()
