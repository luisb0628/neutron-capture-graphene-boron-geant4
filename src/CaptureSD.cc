#include "CaptureSD.hh"
#include "CaptureHit.hh"
#include "RunAction.hh"

#include "G4Step.hh"
#include "G4Track.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"
#include "G4AnalysisManager.hh"
#include "G4TouchableHistory.hh"
#include "G4StepPoint.hh"
#include "G4SDManager.hh"
#include "G4VProcess.hh"
#include "G4RunManager.hh"
#include "G4HadronicProcess.hh"
#include "G4Nucleus.hh"
#include "G4Event.hh"
#include "G4ios.hh"
#include "G4UserRunAction.hh"

// ======================================================
// Constructor
// ======================================================
CaptureSD::CaptureSD(const G4String& name, const G4String& hitsCollectionName)
    : G4VSensitiveDetector(name),
      fHitsCollection(nullptr)
{
    collectionName.insert(hitsCollectionName);
}

// ======================================================
// Destructor necesario para vtable
// ======================================================
CaptureSD::~CaptureSD() {}


// ======================================================
// Inicialización por evento
// ======================================================
void CaptureSD::Initialize(G4HCofThisEvent* hce)
{
    fHitsCollection = new CaptureHitsCollection(SensitiveDetectorName, collectionName[0]);

    auto hcID = G4SDManager::GetSDMpointer()->GetCollectionID(
        SensitiveDetectorName + "/" + collectionName[0]);
    hce->AddHitsCollection(hcID, fHitsCollection);
}


// ======================================================
// ProcessHits — ahora SOLO registra partículas que NACEN
// dentro del volumen del grafeno, usando el vértice
// ======================================================
G4bool CaptureSD::ProcessHits(G4Step* step, G4TouchableHistory*)
{
    auto track = step->GetTrack();
    auto analysis = G4AnalysisManager::Instance();

    // SOLO registrar UNA VEZ por partícula (primer step)
    if (track->GetCurrentStepNumber() != 1)
        return false;


    // SOLO partículas que nacen en el grafeno
    const G4LogicalVolume* vtxLV = track->GetLogicalVolumeAtVertex();
    if (!vtxLV)
        return false;

    G4String volumeName = vtxLV->GetName();

    if (volumeName != "graphene")
        return false;



    // =====================================================
    // Información del vértice
    // =====================================================
    G4double vertexE = track->GetVertexKineticEnergy();
    G4ThreeVector pos = track->GetVertexPosition();
    G4ThreeVector dir = track->GetMomentumDirection();
    G4double time = track->GetGlobalTime();


    auto particle = track->GetDefinition();
    G4String pname = particle->GetParticleName();

    auto eventID  = G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID();
    G4int trackID  = track->GetTrackID();

    // =====================================================
    // Procesos
    // =====================================================
    const G4VProcess* creatorProcess = track->GetCreatorProcess();
    G4String creatorName = creatorProcess ?
                           creatorProcess->GetProcessName() :
                           "primary";

    const G4VProcess* stepProcess = step->GetPostStepPoint()->GetProcessDefinedStep();
    G4String processName = stepProcess ?
                           stepProcess->GetProcessName() :
                           "creation";

    // =====================================================
    // Target
    // =====================================================
    G4int targetZ = -1;
    G4int targetA = -1;

    if (auto had = dynamic_cast<const G4HadronicProcess*>(creatorProcess))
        if (auto nucleus = had->GetTargetNucleus())
        {
            targetZ = nucleus->GetZ_asInt();
            targetA = nucleus->GetA_asInt();
        }

    // =====================================================
    // Etiqueta simplificada
    // =====================================================
    G4String particleLabel = pname;
    if (pname == "gamma") particleLabel = "Gamma";
    else if (pname == "alpha") particleLabel = "Alpha";
    else if (pname == "Li7")   particleLabel = "Li7";
    else if (pname == "Li6")   particleLabel = "Li6";

    // =====================================================
    // GUARDAR EN NTUPLE ProductosCaptura (ID=1)
    // =====================================================
    analysis->FillNtupleIColumn(1, 0, eventID);
    analysis->FillNtupleSColumn(1, 1, particleLabel);
    analysis->FillNtupleDColumn(1, 2, vertexE / MeV);
    analysis->FillNtupleDColumn(1, 3, dir.x());
    analysis->FillNtupleDColumn(1, 4, dir.y());
    analysis->FillNtupleDColumn(1, 5, dir.z());
    analysis->FillNtupleDColumn(1, 6, pos.x() / cm);
    analysis->FillNtupleDColumn(1, 7, pos.y() / cm);
    analysis->FillNtupleDColumn(1, 8, pos.z() / cm);
    analysis->FillNtupleSColumn(1, 9, creatorName);
    analysis->FillNtupleIColumn(1, 10, targetZ);
    analysis->FillNtupleIColumn(1, 11, targetA);

    analysis->AddNtupleRow(1);

    // =====================================================
    // Contar capturas (una alpha = una captura de B-10)
    // =====================================================
    if (pname == "alpha") {
        auto* runAction = static_cast<RunAction*>(
            const_cast<G4UserRunAction*>(
                G4RunManager::GetRunManager()->GetUserRunAction()));
        if (runAction) runAction->IncrementCaptures();
    }

    // =====================================================
    // Escribir al archivo ASCII (misma info que ProductosCaptura)
    // =====================================================
    {
        auto* runAction = static_cast<RunAction*>(
            const_cast<G4UserRunAction*>(
                G4RunManager::GetRunManager()->GetUserRunAction()));
        if (runAction && runAction->outputFile.is_open()) {
            runAction->outputFile
                << eventID << " " << particleLabel << " "
                << vertexE / MeV << " "
                << dir.x() << " " << dir.y() << " " << dir.z() << " "
                << pos.x() / cm << " " << pos.y() / cm << " " << pos.z() / cm << " "
                << creatorName << " " << targetZ << " " << targetA << "\n";
        }
    }

    // =====================================================
    // Crear hit para debugging
    // =====================================================
    auto hit = new CaptureHit();
    hit->SetTrackID(trackID);
    hit->SetParticleName(particleLabel);
    hit->SetKineticEnergy(vertexE);
    hit->SetPos(pos);
    hit->SetTime(time);
    hit->SetProcessName(processName);
    hit->SetVolumeName(volumeName);
    hit->SetTargetZ(targetZ);
    hit->SetTargetA(targetA);

    fHitsCollection->insert(hit);

    return true;
}



// ======================================================
// EndOfEvent
// ======================================================
void CaptureSD::EndOfEvent(G4HCofThisEvent*)
{
    int nHits = fHitsCollection ? fHitsCollection->entries() : 0;
    if (nHits > 0)
        G4cout << "CaptureSD: " << nHits << " vertices registrados." << G4endl;
}

