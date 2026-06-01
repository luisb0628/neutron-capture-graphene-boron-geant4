# Captura de Neutrones en Grafeno Dopado con Boro y Kapton (Geant4)

Simulación Monte Carlo de la **captura de neutrones térmicos** en una estructura de **grafeno dopado con boro (B-10)** sobre soporte de **Kapton** usando Geant4.

Este proyecto es la segunda etapa de una cadena de simulación:

1. **Neutron_Thermalization** — modera neutrones AmBe en parafina y genera `AmBePhaseSpace.root`
2. **Film_graphene** (este repo) — captura los neutrones en el film grafeno/Kapton y genera partículas secundarias
3. **Scintillator_Sipm** — detecta las partículas secundarias en un centellador acoplado a un SiPM

---

## Contenido del repositorio

```
.
├── CMakeLists.txt
├── main.cc
├── make_video.sh                  # Pipeline simulación → video MP4
├── include/
│   ├── DetectorConstruction.hh    # Geometría configurable (orden de capas, espesores)
│   ├── PrimaryGeneratorAction.hh  # Lee AmBePhaseSpace.root o usa pistola de respaldo
│   ├── RunAction.hh               # Escribe generated_particles.txt y transmitted_particles.txt
│   └── TransmittedSD.hh
├── src/
├── macros/
│   ├── run.mac                    # Macro principal (batch)
│   ├── vis1.mac                   # Visualización interactiva
│   ├── vis_video.mac              # Exporta frames EPS por evento
│   └── vis_box_frame.mac          # Frame de geometría para overlay de video
└── python/
    ├── run_simulations.py         # Barrido de espesor de grafeno (1–99 um)
    ├── run_3configs.py            # Corre 3 configuraciones de apilado
    ├── compare_transmitted.py     # Análisis comparativo de las 3 configs
    ├── accumulate_frames.py       # Acumula frames para el video
    ├── analysis.ipynb
    ├── analysis_boron.ipynb
    └── gamma_boron.ipynb
```

---

## Requisitos

- **Geant4** ≥ 10.7 compilado con `ui_all`, `vis_all`, `analysis` y física HP
- **CMake** ≥ 3.16, **C++17**
- **Python 3** con: `uproot`, `numpy`, `pandas`, `matplotlib`, `seaborn`, `jupyter`
- **Ghostscript** (`gs`) y **ffmpeg** — solo para generación de video
- `AmBePhaseSpace.root` generado por **Neutron_Thermalization** (opcional; sin él usa una pistola de neutrones térmicos de respaldo)

---

## Compilación

```bash
source /path/to/geant4/install/bin/geant4.sh
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```

---

## Ejecución

### Modo batch

```bash
cd build
./Film_graphene ../macros/run.mac
```

Genera en `build/`:
- `output.root` — histogramas y ntuples
- `generated_particles.txt` — partículas nacidas en el grafeno (alfa, Li-7, gamma 478 keV)
- `transmitted_particles.txt` — todas las partículas que alcanzan el detector

### Modo interactivo

```bash
cd build
./Film_graphene
```

---

## Comandos del macro

```bash
# Espesores
/detector/grapheneThickness 5 um
/detector/kaptonThickness   125 um
/detector/boronFraction     0.05

# Orden de las capas (respecto al haz)
/detector/order kaptonFirst    # Kapton → Grafeno → Detector (default)
/detector/order grapheneFirst  # Grafeno → Kapton → Detector

# Deshabilitar el grafeno (solo Kapton, como referencia)
/detector/enableGraphene false

/run/initialize
/run/beamOn 100000
```

---

## Fuente de partículas (phase space)

`PrimaryGeneratorAction` lee `AmBePhaseSpace.root` (generado por Neutron_Thermalization) y reproduce el espacio de fase completo de cada partícula: tipo, energía, posición y dirección.  
Si el archivo no existe, usa una pistola de respaldo con neutrones térmicos de 0.025 eV.

---

## Scripts de Python

### Barrido de espesor (`run_simulations.py`)

Corre la simulación variando el espesor del grafeno de 1 a 99 um en pasos de 2 um.

```bash
python python/run_simulations.py
```

Genera `build/output_<X>um.root` por cada espesor.

### 3 configuraciones (`run_3configs.py`)

Corre las tres configuraciones de apilado y guarda los archivos de partículas transmitidas por separado:

| Config | Orden |
|--------|-------|
| `kaptonFirst` | Kapton → Grafeno → Detector |
| `grapheneFirst` | Grafeno → Kapton → Detector |
| `noGraphene` | Solo Kapton → Detector |

```bash
python python/run_3configs.py
```

Genera `build/transmitted_<config>.txt` y `build/output_<config>.root`.

### Análisis comparativo (`compare_transmitted.py`)

Lee los 3 archivos de salida y genera gráficas en `python/plots_3configs/`:
- Conteo y fracción de transmisión por tipo de partícula
- Espectros de energía de neutrones y gammas
- Origen de partículas (VertexVolume)
- Tabla resumen CSV

```bash
python python/compare_transmitted.py
```

---

## Generación de video

El script `make_video.sh` automatiza el pipeline completo:

```bash
./make_video.sh -n 120 -f 60 -o video_grafeno.mp4
```

| Opción | Descripción |
|--------|-------------|
| `-n FRAMES` | número de frames/eventos (default: 120) |
| `-f FPS` | framerate del video (default: 60) |
| `-a ALPHA` | opacidad del overlay de geometría (default: 0.25) |
| `-o OUTPUT` | nombre del MP4 de salida |
| `--no-sim` | reusar EPS existentes |
| `--no-rebuild` | no recompilar |

Dependencias: `gs` (Ghostscript), `ffmpeg`, entorno Python en `~/mi_entorno`.

---

## Estructura del código

| Clase | Responsabilidad |
|-------|----------------|
| `DetectorConstruction` | Geometría: capa grafeno-boro, soporte Kapton, detector; configurable desde macro |
| `PrimaryGeneratorAction` | Lee `AmBePhaseSpace.root`; pistola de respaldo si no existe |
| `RunAction` | Abre/cierra ROOT y archivos ASCII de salida |
| `TransmittedSD` | Detector sensible: registra todas las partículas con phase space |
| `CaptureSD` | Registra capturas neutrónicas en el grafeno |

---

## Contacto

Proyecto desarrollado por **Luis Beltrán**  
Maestría en Ingeniería Física — UAN
