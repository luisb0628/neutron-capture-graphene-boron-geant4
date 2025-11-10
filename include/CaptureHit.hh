#ifndef CAPTUREHIT_HH
#define CAPTUREHIT_HH

#include "G4VHit.hh"
#include "G4ThreeVector.hh"
#include "globals.hh"
#include "G4Allocator.hh"
#include "G4THitsCollection.hh"  // <-- necesario para la colección

class CaptureHit : public G4VHit
{
public:
    CaptureHit();
    virtual ~CaptureHit();
    CaptureHit(const CaptureHit&);
    const CaptureHit& operator=(const CaptureHit&);
    G4bool operator==(const CaptureHit&) const;

    inline void* operator new(size_t);
    inline void operator delete(void*);

    void Print() override;

    // --- Setters ---
    void SetTrackID(G4int id) { fTrackID = id; }
    void SetParticleName(const G4String& name) { fParticleName = name; }
    void SetParticleType(G4int type) { fParticleType = type; }
    void SetEdep(G4double edep) { fEdep = edep; }
    void SetStepLength(G4double length) { fStepLength = length; }
    void SetPos(const G4ThreeVector& pos) { fPos = pos; }
    void SetTime(G4double t) { fTime = t; }
    void SetKineticEnergy(G4double e) { fKineticEnergy = e; }
    void SetProcessName(const G4String& name) { fProcessName = name; }
    void SetVolumeName(const G4String& name) { fVolumeName = name; }
    void SetRegionType(G4int val) { fRegionType = val; } // NUEVO

    // --- Getters ---
    G4int GetTrackID() const { return fTrackID; }
    const G4String& GetParticleName() const { return fParticleName; }
    G4int GetParticleType() const { return fParticleType; }
    G4double GetEdep() const { return fEdep; }
    G4double GetStepLength() const { return fStepLength; }
    const G4ThreeVector& GetPos() const { return fPos; }
    G4double GetTime() const { return fTime; }
    G4double GetKineticEnergy() const { return fKineticEnergy; }
    const G4String& GetProcessName() const { return fProcessName; }
    const G4String& GetVolumeName() const { return fVolumeName; }
    G4int GetRegionType() const { return fRegionType; }

private:
    G4int fTrackID;
    G4String fParticleName;
    G4int fParticleType;
    G4double fEdep;
    G4double fStepLength;
    G4ThreeVector fPos;
    G4double fTime;
    G4double fKineticEnergy;
    G4String fProcessName;
    G4String fVolumeName;
    G4int fRegionType; // 0 = graphene, 1 = capture
};

// --- Allocator global ---
extern G4ThreadLocal G4Allocator<CaptureHit>* CaptureHitAllocator;

inline void* CaptureHit::operator new(size_t)
{
    if (!CaptureHitAllocator)
        CaptureHitAllocator = new G4Allocator<CaptureHit>;
    return (void*)CaptureHitAllocator->MallocSingle();
}

inline void CaptureHit::operator delete(void* hit)
{
    CaptureHitAllocator->FreeSingle((CaptureHit*)hit);
}

// --- Definir el tipo de colección de hits (debe ir después de la clase) ---
typedef G4THitsCollection<CaptureHit> CaptureHitsCollection;

#endif
