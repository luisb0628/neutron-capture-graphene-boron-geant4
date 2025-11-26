#include "TransmittedSD.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4SystemOfUnits.hh"
#include "G4AnalysisManager.hh"
#include "G4ParticleDefinition.hh"
#include "G4TouchableHistory.hh"
#include "G4EventManager.hh"
#include "G4Event.hh"

TransmittedSD::TransmittedSD(const G4String& name)
 : G4VSensitiveDetector(name)
{}

TransmittedSD::~TransmittedSD() = default;

void TransmittedSD::Initialize(G4HCofThisEvent*) {}

G4bool TransmittedSD::ProcessHits(G4Step* aStep, G4TouchableHistory*)
{
    auto post  = aStep->GetPostStepPoint();
    auto pre   = aStep->GetPreStepPoint();
    auto track = aStep->GetTrack();

    // Solo registrar partículas que SALEN del volumen
    if (post->GetStepStatus() != fGeomBoundary)
        return false;

    // ----------------------------------------------------
    // Información general
    // ----------------------------------------------------
    G4String particleName = track->GetDefinition()->GetParticleName();
    G4int eventID   = G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID();
    G4int trackID   = track->GetTrackID();
    G4int parentID  = track->GetParentID();
    G4double kinE   = post->GetKineticEnergy() / MeV;  // eV directamente
    G4ThreeVector pos = post->GetPosition() / mm;
    G4ThreeVector dir = track->GetMomentumDirection();
    G4double time  = post->GetGlobalTime() / ns;
    G4String volumeName = pre->GetPhysicalVolume()->GetName();

    // Creator process
    G4String creatorName = "primary";
    if (track->GetCreatorProcess())
        creatorName = track->GetCreatorProcess()->GetProcessName();

    // ----------------------------------------------------
    // Guardar TODAS las partículas
    // ----------------------------------------------------
    auto analysis = G4AnalysisManager::Instance();

    // Ntuple universal ID=0
    analysis->FillNtupleIColumn(0, 0, eventID);
    analysis->FillNtupleIColumn(0, 1, trackID);
    analysis->FillNtupleIColumn(0, 2, parentID);
    analysis->FillNtupleSColumn(0, 3, particleName);
    analysis->FillNtupleDColumn(0, 4, kinE);
    analysis->FillNtupleDColumn(0, 5, pos.x());
    analysis->FillNtupleDColumn(0, 6, pos.y());
    analysis->FillNtupleDColumn(0, 7, pos.z());
    analysis->FillNtupleDColumn(0, 8, dir.x());
    analysis->FillNtupleDColumn(0, 9, dir.y());
    analysis->FillNtupleDColumn(0, 10, dir.z());
    analysis->FillNtupleDColumn(0, 11, time);
    analysis->FillNtupleSColumn(0, 12, volumeName);
    analysis->FillNtupleSColumn(0, 13, creatorName);

    analysis->AddNtupleRow(0);

    return true;
}

void TransmittedSD::EndOfEvent(G4HCofThisEvent*) {}
