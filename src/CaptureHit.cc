#include "CaptureHit.hh"
#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"

G4ThreadLocal G4Allocator<CaptureHit>* CaptureHitAllocator = nullptr;

CaptureHit::CaptureHit()
    : G4VHit(),
      fTrackID(-1),
      fParticleName(""),
      fParticleType(0),
      fEdep(0.),
      fStepLength(0.),
      fPos(G4ThreeVector()),
      fTime(0.),
      fKineticEnergy(0.),
      fProcessName(""),
      fVolumeName("")
{}

CaptureHit::~CaptureHit()
{}

CaptureHit::CaptureHit(const CaptureHit& right)
    : G4VHit()
{
    fTrackID = right.fTrackID;
    fParticleName = right.fParticleName;
    fParticleType = right.fParticleType;
    fEdep = right.fEdep;
    fStepLength = right.fStepLength;
    fPos = right.fPos;
    fTime = right.fTime;
    fKineticEnergy = right.fKineticEnergy;
    fProcessName = right.fProcessName;
    fVolumeName = right.fVolumeName;
}

const CaptureHit& CaptureHit::operator=(const CaptureHit& right)
{
    fTrackID = right.fTrackID;
    fParticleName = right.fParticleName;
    fParticleType = right.fParticleType;
    fEdep = right.fEdep;
    fStepLength = right.fStepLength;
    fPos = right.fPos;
    fTime = right.fTime;
    fKineticEnergy = right.fKineticEnergy;
    fProcessName = right.fProcessName;
    fVolumeName = right.fVolumeName;
    return *this;
}

G4bool CaptureHit::operator==(const CaptureHit& right) const
{
    return (this == &right) ? true : false;
}

void CaptureHit::Print()
{
    G4cout << "Capture Hit: "
           << " TrackID: " << fTrackID
           << " Particle: " << fParticleName
           << " Type: " << fParticleType
           << " Edep: " << G4BestUnit(fEdep, "Energy")
           << " StepLength: " << G4BestUnit(fStepLength, "Length")
           << " Position: " << G4BestUnit(fPos, "Length")
           << " Time: " << G4BestUnit(fTime, "Time")
           << " KineticEnergy: " << G4BestUnit(fKineticEnergy, "Energy")
           << " Process: " << fProcessName
           << " Volume: " << fVolumeName
           << G4endl;
}