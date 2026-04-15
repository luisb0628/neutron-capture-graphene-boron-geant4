#ifndef CaptureHit_h
#define CaptureHit_h 1

#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"

class CaptureHit : public G4VHit
{
public:
    CaptureHit();
    virtual ~CaptureHit();
    CaptureHit(const CaptureHit&);
    const CaptureHit& operator=(const CaptureHit&);
    G4bool operator==(const CaptureHit&) const;

    inline void* operator new(size_t);
    inline void  operator delete(void*);

    // ==========================
    // Setters
    // ==========================
    void SetTrackID(G4int id)              { fTrackID = id; }
    void SetParticleName(const G4String& s){ fParticleName = s; }
    void SetParticleType(G4int type)       { fParticleType = type; }
    void SetEdep(G4double e)               { fEdep = e; }
    void SetStepLength(G4double l)         { fStepLength = l; }
    void SetPos(const G4ThreeVector& p)    { fPos = p; }
    void SetTime(G4double t)               { fTime = t; }
    void SetKineticEnergy(G4double e)      { fKineticEnergy = e; }
    void SetProcessName(const G4String& p) { fProcessName = p; }
    void SetVolumeName(const G4String& v)  { fVolumeName = v; }
    void SetRegionType(G4int t)            { fRegionType = t; }

    // NUEVOS setters
    void SetTargetZ(G4int Z)               { fTargetZ = Z; }
    void SetTargetA(G4int A)               { fTargetA = A; }

    // ==========================
    // Getters
    // ==========================
    G4int        GetTrackID()        const { return fTrackID; }
    G4String     GetParticleName()   const { return fParticleName; }
    G4int        GetParticleType()   const { return fParticleType; }
    G4double     GetEdep()           const { return fEdep; }
    G4double     GetStepLength()     const { return fStepLength; }
    G4ThreeVector GetPos()           const { return fPos; }
    G4double     GetTime()           const { return fTime; }
    G4double     GetKineticEnergy()  const { return fKineticEnergy; }
    G4String     GetProcessName()    const { return fProcessName; }
    G4String     GetVolumeName()     const { return fVolumeName; }
    G4int        GetRegionType()     const { return fRegionType; }

    // NUEVOS getters
    G4int        GetTargetZ()        const { return fTargetZ; }
    G4int        GetTargetA()        const { return fTargetA; }

    void Print() override;

private:
    // Datos existentes
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
    G4int fRegionType;

    // NUEVOS CAMPOS
    G4int fTargetZ;   // núm. atómico del objetivo
    G4int fTargetA;   // núm. másico del objetivo
};

// =========================================
// Global allocator
// =========================================
extern G4ThreadLocal G4Allocator<CaptureHit>* CaptureHitAllocator;

inline void* CaptureHit::operator new(size_t)
{
    if (!CaptureHitAllocator)
        CaptureHitAllocator = new G4Allocator<CaptureHit>;
    return (void*) CaptureHitAllocator->MallocSingle();
}

inline void CaptureHit::operator delete(void* hit)
{
    CaptureHitAllocator->FreeSingle((CaptureHit*)hit);
}

#endif
