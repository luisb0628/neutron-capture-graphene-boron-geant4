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
      fVolumeName(""),
      fRegionType(-1)
{}

CaptureHit::~CaptureHit() = default;

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
    fRegionType = right.fRegionType;
}

const CaptureHit& CaptureHit::operator=(const CaptureHit& right)
{
    if (this == &right) return *this;
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
    fRegionType = right.fRegionType;
    return *this;
}

G4bool CaptureHit::operator==(const CaptureHit& right) const
{
    return (this == &right);
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
           << " RegionType: " << fRegionType
           << G4endl;
}
