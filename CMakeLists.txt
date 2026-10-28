cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(NeutronBeam)

# --- Encontrar Geant4 ---
find_package(Geant4 REQUIRED ui_all vis_all analysis)

# --- Encontrar ROOT (para leer AmBePhaseSpace.root en PrimaryGeneratorAction) ---
find_package(ROOT REQUIRED COMPONENTS Core RIO Tree)

# --- Incluir directorios de encabezados ---
include(${Geant4_USE_FILE})
include_directories(${PROJECT_SOURCE_DIR}/include)

# --- Archivos fuente ---
set(SOURCES
    src/ActionInitialization.cc
    src/DetectorConstruction.cc
    src/PrimaryGeneratorAction.cc
    src/RunAction.cc
    src/TransmittedSD.cc
    src/EventAction.cc     
    src/CaptureSD.cc
    src/CaptureHit.cc
    src/SteppingAction.cc
    src/PhaseSpaceConfig.cc
    src/GeneratorMessenger.cc
)

# --- Ejecutable principal ---
add_executable(Film_graphene main.cc ${SOURCES})

# --- Enlazar librerías de Geant4 + ROOT ---
target_link_libraries(Film_graphene ${Geant4_LIBRARIES} ROOT::Core ROOT::RIO ROOT::Tree)
target_include_directories(Film_graphene PRIVATE ${ROOT_INCLUDE_DIRS})

# --- Copiar macros automáticamente al build ---
# NOTA 2026-10: antes "${PROJECT_BINARY_DIR}/macros/" (con slash, sin nombre
# de archivo) terminaba creando "macros" como un ARCHIVO (con el contenido
# del ultimo .mac procesado por el glob), no como carpeta -- dependía de que
# CMake ya conociera la carpeta de destino. Se arma la ruta de destino
# completa (carpeta + nombre de archivo) explícitamente para que no dependa
# de esa ambigüedad.
file(GLOB MACRO_FILES "${PROJECT_SOURCE_DIR}/macros/*.mac")
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/macros)
foreach(_file ${MACRO_FILES})
    get_filename_component(_fname ${_file} NAME)
    configure_file(${_file} ${PROJECT_BINARY_DIR}/macros/${_fname} COPYONLY)
endforeach()

# --- Mensajes informativos ---
message(STATUS "Geant4 found at: ${Geant4_DIR}")
message(STATUS "Project built in: ${PROJECT_BINARY_DIR}")

# --- Opcional: instalación ---
install(TARGETS Film_graphene DESTINATION bin)

