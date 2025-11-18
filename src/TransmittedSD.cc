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
    auto post = aStep->GetPostStepPoint();
    auto pre  = aStep->GetPreStepPoint();
    auto track = aStep->GetTrack();

    // Detectar partículas que salen del volumen
    if (post->GetStepStatus() != fGeomBoundary)
        return false;

    // Obtener información general
    G4String particleName = track->GetDefinition()->GetParticleName();
    G4int eventID   = G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID();
    G4int trackID   = track->GetTrackID();
    G4int parentID  = track->GetParentID();
    G4double kinE   = post->GetKineticEnergy() / MeV;
    G4ThreeVector pos = post->GetPosition() / mm;
    G4ThreeVector dir = track->GetMomentumDirection();
    G4double time  = post->GetGlobalTime() / ns;
    G4String volumeName = pre->GetPhysicalVolume()->GetName();
    G4String creatorName = "primary";
    if (track->GetCreatorProcess())
        creatorName = track->GetCreatorProcess()->GetProcessName();

    // Guardar solo partículas relevantes
    if (particleName == "neutron" || particleName == "gamma")
    {
        auto analysisManager = G4AnalysisManager::Instance();

        // --- Histograma neutrones (ID=0) ---
        if (particleName == "gamma")
            analysisManager->FillH1(0, kinE);  // convertir a eV

        // --- Ntuple extendido: NeutronData (ID=0) ---
        analysisManager->FillNtupleIColumn(0, 0, eventID);
        analysisManager->FillNtupleIColumn(0, 1, trackID);
        analysisManager->FillNtupleIColumn(0, 2, parentID);
        analysisManager->FillNtupleSColumn(0, 3, particleName);
        analysisManager->FillNtupleDColumn(0, 4, kinE);
        analysisManager->FillNtupleDColumn(0, 5, pos.x());
        analysisManager->FillNtupleDColumn(0, 6, pos.y());
        analysisManager->FillNtupleDColumn(0, 7, pos.z());
        analysisManager->FillNtupleDColumn(0, 8, dir.x());
        analysisManager->FillNtupleDColumn(0, 9, dir.y());
        analysisManager->FillNtupleDColumn(0, 10, dir.z());
        analysisManager->FillNtupleDColumn(0, 11, time);
        analysisManager->FillNtupleSColumn(0, 12, volumeName);
        analysisManager->FillNtupleSColumn(0, 13, creatorName);

        analysisManager->AddNtupleRow(0);
        return true;
    }

    return false;
}

void TransmittedSD::EndOfEvent(G4HCofThisEvent*) {}
