#include "CaptureSD.hh"
#include "CaptureHit.hh"
#include "RunAction.hh"
#include "G4Step.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"
#include "G4AnalysisManager.hh"
#include "G4TouchableHistory.hh"
#include "G4Track.hh"
#include "G4StepPoint.hh"
#include "G4SDManager.hh"
#include "G4ios.hh"
#include "G4VProcess.hh"
#include "G4RunManager.hh"
#include "G4Event.hh"

CaptureSD::CaptureSD(const G4String& name, const G4String& hitsCollectionName)
    : G4VSensitiveDetector(name), fHitsCollection(nullptr)
{
    collectionName.insert(hitsCollectionName);
}

CaptureSD::~CaptureSD() = default;

void CaptureSD::Initialize(G4HCofThisEvent* hce)
{
    fHitsCollection = new CaptureHitsCollection(SensitiveDetectorName, collectionName[0]);
    G4int hcID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
    hce->AddHitsCollection(hcID, fHitsCollection);
}

G4bool CaptureSD::ProcessHits(G4Step* step, G4TouchableHistory*)
{
    auto analysis = G4AnalysisManager::Instance();
    G4Track* track = step->GetTrack();

    // --- Solo el primer step del track ---
    if (track->GetCurrentStepNumber() != 1)
        return false;

    auto particle = track->GetDefinition();
    auto prePoint = step->GetPreStepPoint();
    auto volumeName = prePoint->GetTouchableHandle()->GetVolume()->GetName();

    G4double energy = step->GetPreStepPoint()->GetKineticEnergy();
    G4double edep   = step->GetTotalEnergyDeposit();
    G4ThreeVector pos = prePoint->GetPosition();
    G4ThreeVector dir = track->GetMomentumDirection();
    G4double time   = track->GetGlobalTime();
    G4double theta  = dir.theta() * 180. / CLHEP::pi;
    G4double stepLength = step->GetStepLength();

    // --- Procesos de creación y del step ---
    const G4VProcess* creator = track->GetCreatorProcess();
    const G4VProcess* process = step->GetPostStepPoint()->GetProcessDefinedStep();
    G4String creatorName = creator ? creator->GetProcessName() : "primary";
    G4String processName = process ? process->GetProcessName() : "Unknown";

    // --- Identificación del tipo de partícula ---
    G4int Z = particle->GetAtomicNumber();
    G4int A = particle->GetAtomicMass();
    G4String pname = particle->GetParticleName();
    G4String particleLabel = "Otro";

    if (pname == "gamma") {
    // Aceptar TODOS los gammas en el grafeno y luego filtrar en análisis
    particleLabel = "Gamma";

    // Energía cinética del fotón en el punto de creación del track
    G4double Ekin_gamma = track->GetVertexKineticEnergy() / MeV;

    // Llenar histograma de energía de gammas con la energía de creación
    analysis->FillH1(3, Ekin_gamma);
    }


    
    else if (Z == 2 && A == 4) {
        particleLabel = "Alpha";
    }
    else if (Z == 3 && (A == 6 || A == 7)) {
        particleLabel = "Litio";
    }
    else {
        return false; // ignorar partículas irrelevantes
    }

    // --- Solo registrar si ocurre en el film de grafeno ---
    if (volumeName != "graphene") return false; 
    G4int regionType = 0;

    // --- Llenar histogramas ---
    if (particleLabel == "Alpha") {
        analysis->FillH1(1, energy);
        analysis->FillH1(4, theta); // distribución angular
    }
    else if (particleLabel == "Litio") {
        analysis->FillH1(2, energy);
    }
    

    // --- IDs de evento y track ---
    auto eventID = G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID();
    G4int trackID = track->GetTrackID();
    G4int parentID = track->GetParentID();

    // --- Llenar ntuple ---
    analysis->FillNtupleIColumn(1, 0, eventID);
    analysis->FillNtupleIColumn(1, 1, trackID);
    analysis->FillNtupleIColumn(1, 2, parentID);
    analysis->FillNtupleSColumn(1, 3, particleLabel);
    analysis->FillNtupleIColumn(1, 4, regionType);
    analysis->FillNtupleDColumn(1, 5, energy / MeV);
    analysis->FillNtupleDColumn(1, 6, edep / MeV);
    analysis->FillNtupleDColumn(1, 7, pos.x() / mm);
    analysis->FillNtupleDColumn(1, 8, pos.y() / mm);
    analysis->FillNtupleDColumn(1, 9, pos.z() / mm);
    analysis->FillNtupleDColumn(1,10, dir.x());
    analysis->FillNtupleDColumn(1,11, dir.y());
    analysis->FillNtupleDColumn(1,12, dir.z());
    analysis->FillNtupleDColumn(1,13, stepLength / mm);
    analysis->FillNtupleDColumn(1,14, time / ns);
    analysis->FillNtupleDColumn(1,15, theta);
    analysis->FillNtupleSColumn(1,16, volumeName);
    analysis->FillNtupleSColumn(1,17, creatorName);
    analysis->FillNtupleSColumn(1,18, processName);
    analysis->AddNtupleRow(1);

    // --- Guardar hit ---
    auto newHit = new CaptureHit();
    newHit->SetTrackID(trackID);
    newHit->SetParticleName(pname);
    newHit->SetEdep(edep);
    newHit->SetStepLength(stepLength);
    newHit->SetPos(pos);
    newHit->SetTime(time);
    newHit->SetKineticEnergy(energy);
    newHit->SetProcessName(processName);
    newHit->SetVolumeName(volumeName);
    newHit->SetRegionType(regionType);
    fHitsCollection->insert(newHit);

    // --- Exportar ASCII ---
    auto runAction = const_cast<RunAction*>(
        static_cast<const RunAction*>(G4RunManager::GetRunManager()->GetUserRunAction()));

    if (runAction && runAction->outputFile.is_open()) {
        runAction->outputFile << eventID << " " << trackID << " " << parentID << " "
                              << particleLabel << " " << energy / MeV << " "
                              << pos.x() / mm << " " << pos.y() / mm << " " << pos.z() / mm << " "
                              << dir.x() << " " << dir.y() << " " << dir.z() << " "
                              << time / ns << " " << creatorName << " " << processName << "\n";
    }

    return true;
}

void CaptureSD::EndOfEvent(G4HCofThisEvent*)
{
    G4int nofHits = fHitsCollection->entries();
    if (nofHits > 0)
        G4cout << "CaptureSD: " << nofHits << " hits in this event" << G4endl;
}
