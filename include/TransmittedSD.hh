#ifndef TransmittedSD_h
#define TransmittedSD_h 1

#include "G4VSensitiveDetector.hh"
#include "globals.hh"

class TransmittedSD : public G4VSensitiveDetector
{
public:
    explicit TransmittedSD(const G4String& name);
    ~TransmittedSD() override;

    void Initialize(G4HCofThisEvent*) override;
    G4bool ProcessHits(G4Step*, G4TouchableHistory*) override;
    void EndOfEvent(G4HCofThisEvent*) override;
};

#endif
