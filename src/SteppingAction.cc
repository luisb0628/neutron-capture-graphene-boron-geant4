#include "SteppingAction.hh"

#include "G4Step.hh"
#include "G4Track.hh"
#include "G4RunManager.hh"
#include "G4AnalysisManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4LogicalVolume.hh"

SteppingAction::SteppingAction() {}
SteppingAction::~SteppingAction() {}

void SteppingAction::UserSteppingAction(const G4Step* step)
{
    auto track = step->GetTrack();

    // Solo secundarios
    if (track->GetParentID() == 0)
        return;

    // Solo alpha o litio
    G4String pname = track->GetDefinition()->GetParticleName();
    if (pname != "alpha" && pname != "Li7" && pname != "Li6")
        return;

    // Solo partículas que nacieron en el grafeno
    const G4LogicalVolume* vtxLV = track->GetLogicalVolumeAtVertex();
    if (!vtxLV)
        return;

    if (vtxLV->GetName() != "graphene")
        return;

    // Registrar solo cuando la partícula muere
    if (track->GetTrackStatus() != fStopAndKill)
        return;

    // ===============================
    // Guardar en NTUPLE RangoIones (ID = 2)
    // ===============================
    auto analysis = G4AnalysisManager::Instance();

    G4int eventID = G4RunManager::GetRunManager()
                        ->GetCurrentEvent()->GetEventID();

    G4double vertexE        = track->GetVertexKineticEnergy();
    G4double totalTrackLength = track->GetTrackLength();

    const G4VProcess* creator = track->GetCreatorProcess();
    G4String creatorName = creator ? creator->GetProcessName() : "primary";

    analysis->FillNtupleIColumn(2, 0, eventID);
    analysis->FillNtupleSColumn(2, 1, pname);
    analysis->FillNtupleDColumn(2, 2, vertexE / MeV);
    analysis->FillNtupleDColumn(2, 3, totalTrackLength / um);
    analysis->FillNtupleSColumn(2, 4, creatorName);

    analysis->AddNtupleRow(2);
}
