#include "TransmittedSD.hh"

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
    if (particleName == "neutron")
        return false;

    G4int eventID = G4EventManager::GetEventManager()
                        ->GetConstCurrentEvent()->GetEventID();

    G4double kinE = post->GetKineticEnergy() / keV;
    G4ThreeVector dir = track->GetMomentumDirection();

    // Volumen donde nació la partícula: identifica si vino del grafeno o del Kapton
    const G4LogicalVolume* vtxLV = track->GetLogicalVolumeAtVertex();
    G4String vertexVolume = vtxLV ? vtxLV->GetName() : "unknown";

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
    analysis->FillNtupleSColumn(0, 6, vertexVolume);   // origen: "graphene" o "kapton"
    analysis->FillNtupleSColumn(0, 7, creatorName);
    analysis->FillNtupleIColumn(0, 8, targetZ);
    analysis->FillNtupleIColumn(0, 9, targetA);

    analysis->AddNtupleRow(0);

    return true;
}

void TransmittedSD::EndOfEvent(G4HCofThisEvent*) {}
