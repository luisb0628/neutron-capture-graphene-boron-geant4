#include "CaptureSD.hh"
#include "G4Step.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"
#include "G4AnalysisManager.hh"
#include "G4TouchableHistory.hh"
#include "G4Track.hh"
#include "G4StepPoint.hh"
#include "G4SDManager.hh"
#include "G4ios.hh"
#include "G4VProcess.hh"  // ¡NECESARIO PARA USAR G4VProcess!

CaptureSD::CaptureSD(const G4String& name, const G4String& hitsCollectionName)
    : G4VSensitiveDetector(name),
      fHitsCollection(nullptr)
{
    collectionName.insert(hitsCollectionName);
}

CaptureSD::~CaptureSD()
{
}

void CaptureSD::Initialize(G4HCofThisEvent* hce)
{
    // Crear la colección de hits
    fHitsCollection = new CaptureHitsCollection(SensitiveDetectorName, collectionName[0]);
    
    G4int hcID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
    hce->AddHitsCollection(hcID, fHitsCollection);
}

G4bool CaptureSD::ProcessHits(G4Step* step, G4TouchableHistory*)
{
    auto analysisManager = G4AnalysisManager::Instance();
    
    G4Track* track = step->GetTrack();
    G4String particleName = track->GetDefinition()->GetParticleName();
    G4double energy = track->GetKineticEnergy();
    G4double edep = step->GetTotalEnergyDeposit();
    G4ThreeVector pos = step->GetPostStepPoint()->GetPosition();
    G4double time = track->GetGlobalTime();
    G4double theta = pos.theta(); // en radianes

    G4bool isParticleOfInterest = false;
    G4int particleType = 0;

    // Detectar iones de litio
    if (particleName == "Li7" || particleName == "Li6") {
        isParticleOfInterest = true;
        particleType = 1; // Litio
        
        // Llenar histograma de energía de litio (ID 3)
        analysisManager->FillH1(3, energy);
        
        // Llenar ntuple de captura
        analysisManager->FillNtupleIColumn(1, 0, particleType);
        analysisManager->FillNtupleDColumn(1, 1, energy/MeV);
        analysisManager->FillNtupleDColumn(1, 2, edep/MeV);
        analysisManager->FillNtupleDColumn(1, 3, pos.x()/mm);
        analysisManager->FillNtupleDColumn(1, 4, pos.y()/mm);
        analysisManager->FillNtupleDColumn(1, 5, pos.z()/mm);
        analysisManager->FillNtupleDColumn(1, 6, time/ns);
        analysisManager->FillNtupleDColumn(1, 7, theta * 180./M_PI);
        analysisManager->FillNtupleSColumn(1, 8, step->GetPreStepPoint()->GetTouchableHandle()->GetVolume()->GetName());
        
        const G4VProcess* process = step->GetPostStepPoint()->GetProcessDefinedStep();
        if (process) {
            analysisManager->FillNtupleSColumn(1, 9, process->GetProcessName());
        } else {
            analysisManager->FillNtupleSColumn(1, 9, "Unknown");
        }
        
        analysisManager->AddNtupleRow(1);
    }
    // Detectar partículas alfa
    else if (particleName == "alpha" || particleName == "He4") {
        isParticleOfInterest = true;
        particleType = 2; // Alpha
        
        // Llenar histogramas de alfa
        analysisManager->FillH1(2, energy); // AlphaEnergy (ID 2)
        analysisManager->FillH1(5, theta * 180./M_PI); // AlphaTheta (ID 5)
        
        // Llenar ntuple de captura
        analysisManager->FillNtupleIColumn(1, 0, particleType);
        analysisManager->FillNtupleDColumn(1, 1, energy/MeV);
        analysisManager->FillNtupleDColumn(1, 2, edep/MeV);
        analysisManager->FillNtupleDColumn(1, 3, pos.x()/mm);
        analysisManager->FillNtupleDColumn(1, 4, pos.y()/mm);
        analysisManager->FillNtupleDColumn(1, 5, pos.z()/mm);
        analysisManager->FillNtupleDColumn(1, 6, time/ns);
        analysisManager->FillNtupleDColumn(1, 7, theta * 180./M_PI);
        analysisManager->FillNtupleSColumn(1, 8, step->GetPreStepPoint()->GetTouchableHandle()->GetVolume()->GetName());
        
        const G4VProcess* process = step->GetPostStepPoint()->GetProcessDefinedStep();
        if (process) {
            analysisManager->FillNtupleSColumn(1, 9, process->GetProcessName());
        } else {
            analysisManager->FillNtupleSColumn(1, 9, "Unknown");
        }
        
        analysisManager->AddNtupleRow(1);
    }
    // Detectar rayos gamma
    else if (particleName == "gamma") {
        isParticleOfInterest = true;
        particleType = 3; // Gamma
        
        // Llenar histograma de gamma (ID 4)
        analysisManager->FillH1(4, energy);
        
        // Llenar ntuple de captura
        analysisManager->FillNtupleIColumn(1, 0, particleType);
        analysisManager->FillNtupleDColumn(1, 1, energy/MeV);
        analysisManager->FillNtupleDColumn(1, 2, edep/MeV);
        analysisManager->FillNtupleDColumn(1, 3, pos.x()/mm);
        analysisManager->FillNtupleDColumn(1, 4, pos.y()/mm);
        analysisManager->FillNtupleDColumn(1, 5, pos.z()/mm);
        analysisManager->FillNtupleDColumn(1, 6, time/ns);
        analysisManager->FillNtupleDColumn(1, 7, theta * 180./M_PI);
        analysisManager->FillNtupleSColumn(1, 8, step->GetPreStepPoint()->GetTouchableHandle()->GetVolume()->GetName());
        
        const G4VProcess* process = step->GetPostStepPoint()->GetProcessDefinedStep();
        if (process) {
            analysisManager->FillNtupleSColumn(1, 9, process->GetProcessName());
        } else {
            analysisManager->FillNtupleSColumn(1, 9, "Unknown");
        }
        
        analysisManager->AddNtupleRow(1);
    }

    // Llenar histograma de tiempo para todas las partículas (ID 6)
    if (isParticleOfInterest) {
        analysisManager->FillH1(6, time);
        
        // Crear hit para la colección
        CaptureHit* newHit = new CaptureHit();
        newHit->SetTrackID(track->GetTrackID());
        newHit->SetParticleName(particleName);
        newHit->SetParticleType(particleType);
        newHit->SetEdep(edep);
        newHit->SetStepLength(step->GetStepLength());
        newHit->SetPos(pos);
        newHit->SetTime(time);
        newHit->SetKineticEnergy(energy);
        
        const G4VProcess* process = step->GetPostStepPoint()->GetProcessDefinedStep();
        if (process) {
            newHit->SetProcessName(process->GetProcessName());
        }
        
        G4TouchableHandle touchable = step->GetPreStepPoint()->GetTouchableHandle();
        newHit->SetVolumeName(touchable->GetVolume()->GetName());
        
        fHitsCollection->insert(newHit);
        
        return true;
    }

    return false;
}

void CaptureSD::EndOfEvent(G4HCofThisEvent*)
{
    // Opcional: imprimir información de depuración
    G4int nofHits = fHitsCollection->entries();
    if (nofHits > 0) {
        G4cout << "CaptureSD: " << nofHits << " hits in this event" << G4endl;
    }
}