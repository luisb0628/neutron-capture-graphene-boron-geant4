#include "GeneratorMessenger.hh"
#include "PhaseSpaceConfig.hh"

#include "G4UIdirectory.hh"
#include "G4UIcmdWithAString.hh"

GeneratorMessenger::GeneratorMessenger() : G4UImessenger()
{
    fGeneratorDir = new G4UIdirectory("/generator/");
    fGeneratorDir->SetGuidance("Comandos para configurar la fuente de primarios (phase space).");

    fPhaseSpaceFileCmd = new G4UIcmdWithAString("/generator/phaseSpaceFile", this);
    fPhaseSpaceFileCmd->SetGuidance(
        "Ruta del .root de phase space (ntuple 'PhaseSpace') que alimenta los "
        "primarios de este proyecto. Default: AmBePhaseSpace.root (neutrones). "
        "Usar Cs137PhaseSpace.root para correr la misma geometria con la fuente "
        "de calibracion (gammas de 661.657 keV) y caracterizar su contribucion "
        "por separado -- ver neutron-thermalization-geant4/macros/{ambe,cs137}_only.mac.");
    fPhaseSpaceFileCmd->SetParameterName("path", false);
    fPhaseSpaceFileCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
}

GeneratorMessenger::~GeneratorMessenger()
{
    delete fPhaseSpaceFileCmd;
    delete fGeneratorDir;
}

void GeneratorMessenger::SetNewValue(G4UIcommand* command, G4String newValue)
{
    if (command == fPhaseSpaceFileCmd) {
        PhaseSpaceConfig::SetFilePath(newValue);
    }
}
