#include "TransmittedSD.hh"
#include "RunAction.hh"

#include "G4Step.hh"
#include "G4Track.hh"
#include "G4LogicalVolume.hh"
#include "G4SystemOfUnits.hh"
#include "G4AnalysisManager.hh"
#include "G4ParticleDefinition.hh"
#include "G4EventManager.hh"
#include "G4Event.hh"
#include "G4VProcess.hh"
#include "G4HadronicProcess.hh"
#include "G4Nucleus.hh"
#include "G4RunManager.hh"
#include "G4UserRunAction.hh"

// ======================================================
// Constructor / Destructor
// ======================================================
TransmittedSD::TransmittedSD(const G4String& name)
    : G4VSensitiveDetector(name)
{}

TransmittedSD::~TransmittedSD() = default;

void TransmittedSD::Initialize(G4HCofThisEvent*) {}


// ======================================================
// ProcessHits
// ======================================================
G4bool TransmittedSD::ProcessHits(G4Step* step, G4TouchableHistory*)
{
    auto post  = step->GetPostStepPoint();
    auto pre   = step->GetPreStepPoint();
    auto track = step->GetTrack();

    // ----------------------------------------------------
    // Registrar solo partículas que ENTRAN al volumen
    // ----------------------------------------------------
    if (pre->GetStepStatus() != fGeomBoundary)
        return false;


    // ====================================================
    // Información general de la partícula
    // ====================================================
    G4String particleName = track->GetDefinition()->GetParticleName();

    G4int eventID = G4EventManager::GetEventManager()
                        ->GetConstCurrentEvent()->GetEventID();

    G4double kinE = post->GetKineticEnergy() / MeV;
    G4ThreeVector pos = pre->GetPosition();
    G4ThreeVector dir = track->GetMomentumDirection();

    // Proceso creador
    G4String creatorName = "primary";
    if (track->GetCreatorProcess())
        creatorName = track->GetCreatorProcess()->GetProcessName();

    // Núcleo objetivo (si hubo interacción hadrónica)
    G4int targetZ = -1;
    G4int targetA = -1;
    if (auto had = dynamic_cast<const G4HadronicProcess*>(track->GetCreatorProcess())) {
        if (auto nucleus = had->GetTargetNucleus()) {
            targetZ = nucleus->GetZ_asInt();
            targetA = nucleus->GetA_asInt();
        }
    }

    // ====================================================
    // Guardar en NTUPLE ID=0 (ParticulasDetector)
    // ====================================================
    auto analysis = G4AnalysisManager::Instance();

    analysis->FillNtupleIColumn(0, 0, eventID);
    analysis->FillNtupleSColumn(0, 1, particleName);
    analysis->FillNtupleDColumn(0, 2, kinE);
    analysis->FillNtupleDColumn(0, 3, dir.x());
    analysis->FillNtupleDColumn(0, 4, dir.y());
    analysis->FillNtupleDColumn(0, 5, dir.z());
    analysis->FillNtupleDColumn(0, 6, pos.x() / cm);
    analysis->FillNtupleDColumn(0, 7, pos.y() / cm);
    analysis->FillNtupleDColumn(0, 8, pos.z() / cm);
    analysis->FillNtupleSColumn(0, 9, creatorName);
    analysis->FillNtupleIColumn(0, 10, targetZ);
    analysis->FillNtupleIColumn(0, 11, targetA);

    analysis->AddNtupleRow(0);

    // ====================================================
    // Escribir al archivo ASCII transmitted_particles.txt
    // ====================================================
    {
        auto* runAction = static_cast<RunAction*>(
            const_cast<G4UserRunAction*>(
                G4RunManager::GetRunManager()->GetUserRunAction()));
        if (runAction && runAction->transmittedFile.is_open()) {
            runAction->transmittedFile
                << eventID << " " << particleName << " "
                << kinE << " "
                << dir.x() << " " << dir.y() << " " << dir.z() << " "
                << pos.x() / cm << " " << pos.y() / cm << " " << pos.z() / cm << " "
                << creatorName << " " << targetZ << " " << targetA << "\n";
        }
    }

    return true;
}

void TransmittedSD::EndOfEvent(G4HCofThisEvent*) {}
