#ifndef CaptureHit_h
#define CaptureHit_h 1

#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"
#include "G4String.hh"

class CaptureHit : public G4VHit
{
public:
    CaptureHit();
    virtual ~CaptureHit();
    CaptureHit(const CaptureHit& right);
    const CaptureHit& operator=(const CaptureHit& right);
    G4bool operator==(const CaptureHit& right) const;
    
    inline void* operator new(size_t);
    inline void operator delete(void* hit);
    
    virtual void Print();
    
    // Setters
    void SetTrackID(G4int id) { fTrackID = id; }
    void SetParticleName(G4String name) { fParticleName = name; }
    void SetParticleType(G4int type) { fParticleType = type; }
    void SetEdep(G4double de) { fEdep = de; }
    void SetStepLength(G4double sl) { fStepLength = sl; }
    void SetPos(G4ThreeVector xyz) { fPos = xyz; }
    void SetTime(G4double t) { fTime = t; }
    void SetKineticEnergy(G4double ke) { fKineticEnergy = ke; }
    void SetProcessName(G4String process) { fProcessName = process; }
    void SetVolumeName(G4String volume) { fVolumeName = volume; }
    
    // Getters
    G4int GetTrackID() const { return fTrackID; }
    G4String GetParticleName() const { return fParticleName; }
    G4int GetParticleType() const { return fParticleType; }
    G4double GetEdep() const { return fEdep; }
    G4double GetStepLength() const { return fStepLength; }
    G4ThreeVector GetPos() const { return fPos; }
    G4double GetTime() const { return fTime; }
    G4double GetKineticEnergy() const { return fKineticEnergy; }
    G4String GetProcessName() const { return fProcessName; }
    G4String GetVolumeName() const { return fVolumeName; }
    
private:
    G4int fTrackID;
    G4String fParticleName;
    G4int fParticleType; // 1=Litio, 2=Alpha, 3=Gamma
    G4double fEdep;
    G4double fStepLength;
    G4ThreeVector fPos;
    G4double fTime;
    G4double fKineticEnergy;
    G4String fProcessName;
    G4String fVolumeName;
};

// Tipo de colección de hits
typedef G4THitsCollection<CaptureHit> CaptureHitsCollection;

extern G4ThreadLocal G4Allocator<CaptureHit>* CaptureHitAllocator;

inline void* CaptureHit::operator new(size_t)
{
    if (!CaptureHitAllocator)
        CaptureHitAllocator = new G4Allocator<CaptureHit>;
    return (void*)CaptureHitAllocator->MallocSingle();
}

inline void CaptureHit::operator delete(void* hit)
{
    CaptureHitAllocator->FreeSingle((CaptureHit*) hit);
}

#endif