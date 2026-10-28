#ifndef PhaseSpaceConfig_h
#define PhaseSpaceConfig_h 1

#include "globals.hh"

// Ruta del archivo .root de phase space que alimenta PrimaryGeneratorAction.
// Desde que neutron-thermalization-geant4 separo AmBe y Cs-137 en dos
// simulaciones independientes (ver /source/mode alli), este proyecto puede
// alimentarse de cualquiera de los dos .root resultantes, uno por corrida:
//   AmBePhaseSpace.root  -> neutrones AmBe (la fisica de interes: captura en
//                           boro-grafeno necesita neutrones, no gammas)
//   Cs137PhaseSpace.root -> gammas de 661.657 keV (para caracterizar el fondo
//                           que la fuente de calibracion Cs-137 aporta al
//                           pasar por la misma pila de peliculas)
// Estado simple de proceso (no por-hilo), igual que SourceMode.hh en el
// proyecto hermano: se fija una sola vez desde el macro, antes de
// /run/initialize.
namespace PhaseSpaceConfig {
    void SetFilePath(const G4String& path);
    const G4String& GetFilePath();
}

#endif
