#ifndef TransmittedSD_h
#define TransmittedSD_h 1

#include "G4VSensitiveDetector.hh"

class G4Step;
class G4HCofThisEvent;

class TransmittedSD : public G4VSensitiveDetector
{
public:
    TransmittedSD(const G4String& name);
    virtual ~TransmittedSD();
    
    virtual void Initialize(G4HCofThisEvent* hitCollection);
    virtual G4bool ProcessHits(G4Step* step, G4TouchableHistory* history);
    virtual void EndOfEvent(G4HCofThisEvent* hitCollection);
    
private:
    // No hay colección de hits porque no usamos TransmittedHit
};

#endif