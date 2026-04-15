#ifndef CaptureSD_h
#define CaptureSD_h 1

#include "G4VSensitiveDetector.hh"
#include "G4THitsCollection.hh"
#include "G4Step.hh"
#include "G4String.hh"
#include "CaptureHit.hh"

// ==========================================================
// Sensitive Detector para registrar partículas que NACEN
// en el volumen del grafeno (vertex-based)
// ==========================================================

class CaptureSD : public G4VSensitiveDetector
{
public:
    CaptureSD(const G4String& name, const G4String& hitsCollectionName);
    virtual ~CaptureSD();

    // Inicialización por evento
    virtual void Initialize(G4HCofThisEvent* hce) override;

    // Captura de información: SOLO en step 1 y SOLO si nació en grafeno
    virtual G4bool ProcessHits(G4Step* step, G4TouchableHistory*) override;

    // Impresión opcional al final del evento
    virtual void EndOfEvent(G4HCofThisEvent* hce) override;

private:

    // Alias simplificado para la colección de hits
    using CaptureHitsCollection = G4THitsCollection<CaptureHit>;

    // Colección de hits del evento
    CaptureHitsCollection* fHitsCollection;
};

#endif
