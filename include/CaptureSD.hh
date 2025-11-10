#ifndef CaptureSD_h
#define CaptureSD_h 1

#include "G4VSensitiveDetector.hh"
#include "CaptureHit.hh"
#include <vector>

class G4Step;
class G4HCofThisEvent;

class CaptureSD : public G4VSensitiveDetector {
public:
    CaptureSD(const G4String& name, const G4String& hitsCollectionName);
    ~CaptureSD() override;

    void Initialize(G4HCofThisEvent* hce) override;
    G4bool ProcessHits(G4Step* step, G4TouchableHistory* history) override;
    void EndOfEvent(G4HCofThisEvent* hce) override;

private:
    CaptureHitsCollection* fHitsCollection;  // <-- ahora sí reconocido
};


#endif