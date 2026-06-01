#ifndef PrimaryGeneratorAction_h
#define PrimaryGeneratorAction_h 1

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"
#include "globals.hh"
#include <vector>
#include <string>

class G4Event;

struct PhaseSpaceEntry {
    std::string particle;
    double kinE_eV;
    double posX_cm, posY_cm;   // posZ se ignora; se reemplaza por startZ
    double dirX, dirY, dirZ;
};

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
public:
    PrimaryGeneratorAction();
    ~PrimaryGeneratorAction() override;
    void GeneratePrimaries(G4Event* event) override;

private:
    void LoadPhaseSpace(const G4String& filename);

    std::vector<PhaseSpaceEntry> fEntries;
    G4int    fCurrentEntry = 0;
    G4bool   fUseFallback  = false;
    G4ParticleGun* fFallbackGun = nullptr;
};

#endif
