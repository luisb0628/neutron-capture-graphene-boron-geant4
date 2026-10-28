#include "PhaseSpaceConfig.hh"

namespace {
    // Default: mismo comportamiento que antes de agregar esta configuracion
    // (apunta a la fuente AmBe, la fisica principal de este proyecto).
    G4String gFilePath = "../../neutron-thermalization-geant4/build/AmBePhaseSpace.root";
}

void PhaseSpaceConfig::SetFilePath(const G4String& path) { gFilePath = path; }

const G4String& PhaseSpaceConfig::GetFilePath() { return gFilePath; }
