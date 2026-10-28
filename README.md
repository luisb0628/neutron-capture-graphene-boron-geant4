# Captura de neutrones en grafeno dopado con boro sobre Kapton (Geant4)

Simulación Monte Carlo en **Geant4** de la captura de neutrones térmicos en una película de **grafeno dopado con ¹⁰B** sobre un soporte de **Kapton**. Los primarios son las partículas que llegan al detector en la etapa de termalización. El programa registra las capturas ¹⁰B(n,α)⁷Li, los productos que nacen en el grafeno y todas las partículas que salen de la película hacia el detector.

Es la **segunda etapa** de una cadena de tres simulaciones:

| Etapa | Repositorio | Entrada | Salida |
|---|---|---|---|
| 1. Termalización | [neutron-thermalization-geant4](https://github.com/luisb0628/neutron-thermalization-geant4) | — | `AmBePhaseSpace.root`, `Cs137PhaseSpace.root` |
| 2. Captura | **neutron-capture-graphene-boron-geant4** (este) | phase space de la etapa 1 | `output.root` |
| 3. Centelleador | [Scintillator_Sipm](https://github.com/luisb0628/Scintillator_Sipm) | `output.root` de esta etapa | `output.root` |

---

## Física y geometría

**Lista de física:** `QGSP_BERT_HP`, la misma de las tres etapas.

**Reacción de interés.** ¹⁰B(n,α)⁷Li produce un α y un ⁷Li y, en el 93,8 % de las capturas, un γ de 0,4776 MeV de la desexcitación del ⁷Li.

**Geometría.** Discos con el eje en *z*, dentro de un mundo de aire de 20 cm de lado:
- Kapton de radio 1,5 cm (espesor configurable, 125 µm por defecto en las macros).
- Grafeno dopado con boro de radio 1,3 cm. Se configuran el espesor, la fracción másica de boro (enriquecido al 80 % en ¹⁰B), la densidad y una oxidación opcional (O e H).
- Detector de aire de radio 1,3 cm, justo detrás de la película.
- Orden de las capas configurable: Kapton primero (por defecto) o grafeno primero. El grafeno se puede quitar para tener el caso de referencia sin película o solo con Kapton.

**Primarios.** Cada evento es una partícula de la tabla `PhaseSpace` de la etapa 1. Se descartan las que van hacia atrás y el resto se recorre de forma cíclica. Cada una nace 1 µm antes de la cara frontal de la película, con su energía, su dirección y su posición (*x*, *y*) en el detector de la etapa 1. Si no se encuentra el archivo, se usa una pistola de respaldo de neutrones térmicos de 0,025 eV.

---

## Requisitos

- **Geant4 11** con datos de neutrones HP, `analysis` y, para visualizar, `ui_all` y `vis_all`.
- **CMake** ≥ 3.16 y un compilador **C++17**.
- **Python 3** con `uproot`, `numpy` y `pandas`, para leer las salidas y correr los barridos.
- El repositorio de la etapa 1 en la misma carpeta que este, con su `build/` ya generado.

Las corridas de la tesis se hicieron en Docker con la imagen `luisb28/geant4-clase:11.4-python`, montando la carpeta que contiene los tres repositorios en `/workspace`.

---

## Compilación y ejecución

```bash
source /ruta/a/geant4/bin/geant4.sh
mkdir -p build && cd build
cmake ..
make -j$(nproc)

./Film_graphene ../macros/run_g.mac     # Kapton + grafeno con boro, primarios del AmBe
./Film_graphene                         # modo interactivo con visualización
```

El programa usa varios hilos (`G4MTRunManager`) y siempre escribe `output.root` en la carpeta donde se ejecuta. Para conservar varias corridas, renombra el archivo después de cada una. La etapa 3 espera, por ejemplo, `output_run_g.root` y `output_run.root`.

---

## Macros

| Macro | Primarios | Película |
|---|---|---|
| `macros/run_g.mac` | AmBe (`AmBePhaseSpace.root`) | Kapton + grafeno con boro |
| `macros/run.mac` | AmBe | solo Kapton (sin grafeno) |
| `macros/run_cs137_g.mac` | ¹³⁷Cs (`Cs137PhaseSpace.root`) | Kapton + grafeno con boro |
| `macros/run_cs137.mac` | ¹³⁷Cs | solo Kapton |
| `macros/vis1.mac`, `vis_*.mac` | — | visualización y video |

Las macros usan 1×10⁷ primarios, un grafeno de 39 µm con 5 % de boro y 1,8 g/cm³, y un Kapton de 125 µm. La película de referencia de la tesis es de **50 µm con 15 % de boro**. La fijan los scripts de las corridas finales con los comandos de abajo.

### Comandos

| Comando | Descripción |
|---|---|
| `/generator/phaseSpaceFile <ruta>` | Archivo de phase space de la etapa 1 (por defecto `../../neutron-thermalization-geant4/build/AmBePhaseSpace.root`) |
| `/detector/grapheneThickness <valor> um` | Espesor del grafeno |
| `/detector/kaptonThickness <valor> um` | Espesor del Kapton |
| `/detector/boronFraction <0–1>` | Fracción másica de boro en el grafeno |
| `/detector/grapheneDensity <valor> g/cm3` | Densidad del grafeno |
| `/detector/oxygenFraction`, `/detector/hydrogenFraction` | Oxidación opcional del grafeno (0 = prístino) |
| `/detector/order kaptonFirst \| grapheneFirst` | Orden de las capas respecto a la fuente |
| `/detector/enableGraphene 0 \| 1` | Quita o pone la capa de grafeno |

---

## Salida (`output.root`)

| Tabla | Contenido |
|---|---|
| `ParticulasTransmitidas` | Cada partícula que entra al detector, es decir, que sale de la película: `EventID`, `ParticleType`, `KineticEnergy_MeV`, `DirX/Y/Z`, `PosX/Y/Z_cm`, `CreatorProcess`, `TargetZ`, `TargetA` y `VertexVolume` (volumen donde nació). Es la entrada de la etapa 3. |
| `ProductosCaptura` | Las partículas que nacen en el grafeno (α, ⁷Li, γ), con las mismas columnas salvo `VertexVolume`. |
| `RangoIones` | Energía de creación (`VertexEnergy_MeV`) y longitud de la trayectoria (`TrackLength_um`) de cada α y ⁷Li. |

---

## Estructura del código

| Clase | Responsabilidad |
|---|---|
| `DetectorConstruction` | Materiales (boro enriquecido, Kapton, grafeno con oxidación opcional), capas y detector; comandos `/detector/` |
| `PrimaryGeneratorAction` | Lee la tabla `PhaseSpace` de la etapa 1, o usa la pistola de respaldo |
| `PhaseSpaceConfig` / `GeneratorMessenger` | Ruta del archivo de phase space (`/generator/phaseSpaceFile`) |
| `CaptureSD` / `CaptureHit` | Registro de las capturas y de los productos en el grafeno |
| `TransmittedSD` | Detector sensible que llena `ParticulasTransmitidas` |
| `RunAction` / `EventAction` / `SteppingAction` | Archivo ROOT, tablas, rango de los iones y progreso |

**Scripts:** `python/run_simulations.py` y `python/run_simulations_boron.py` (barridos de espesor y de fracción de boro), `python/run_3configs.py` y `python/compare_transmitted.py` (comparación de los órdenes de capas), y `make_video.sh` con `python/accumulate_frames.py` (video de la simulación). En `python/plots_*` quedan figuras de estudios anteriores.

---

## Autor

**Luis Beltrán** — Maestría en Ingeniería Física, Universidad Antonio Nariño (UAN).
