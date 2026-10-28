#ifndef GeneratorMessenger_h
#define GeneratorMessenger_h 1

#include "G4UImessenger.hh"
#include "globals.hh"

class G4UIdirectory;
class G4UIcmdWithAString;

// Comando de macro para elegir que archivo de phase space alimenta
// PrimaryGeneratorAction (ver PhaseSpaceConfig.hh). Se construye una sola
// vez, en ActionInitialization (eager, antes de /run/initialize) -- no
// dentro de PrimaryGeneratorAction, que recien se construye en Build() y
// para entonces el macro ya intento usar el comando.
class GeneratorMessenger : public G4UImessenger {
public:
    GeneratorMessenger();
    ~GeneratorMessenger() override;
    void SetNewValue(G4UIcommand* command, G4String newValue) override;

private:
    G4UIdirectory*       fGeneratorDir;
    G4UIcmdWithAString*  fPhaseSpaceFileCmd; // /generator/phaseSpaceFile <ruta>
};

#endif
