cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(NeutronBeam)

# --- Encontrar Geant4 ---
find_package(Geant4 REQUIRED ui_all vis_all analysis)

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
)

# --- Ejecutable principal ---
add_executable(Film_graphene main.cc ${SOURCES})

# --- Enlazar librerías de Geant4 ---
target_link_libraries(Film_graphene ${Geant4_LIBRARIES})

# --- Copiar macros automáticamente al build ---
file(GLOB MACRO_FILES "${PROJECT_SOURCE_DIR}/macros/*.mac")
foreach(_file ${MACRO_FILES})
    configure_file(${_file} ${PROJECT_BINARY_DIR}/macros/ COPYONLY)
endforeach()

# --- Mensajes informativos ---
message(STATUS "Geant4 found at: ${Geant4_DIR}")
message(STATUS "Project built in: ${PROJECT_BINARY_DIR}")

# --- Opcional: instalación ---
install(TARGETS Film_graphene DESTINATION bin)

