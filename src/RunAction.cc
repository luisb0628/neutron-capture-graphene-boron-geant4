#include "RunAction.hh"
#include "EventAction.hh"
#include "G4Run.hh"
#include "G4AnalysisManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"
#include "G4AutoLock.hh"
#include "G4ios.hh"
#include <iomanip>

G4Mutex RunAction::sCaptureMutex   = G4MUTEX_INITIALIZER;
G4int   RunAction::sTotalCaptures  = 0;

RunAction::RunAction() : G4UserRunAction()
{
    // En modo MT cada hilo trabajador escribe su propio archivo temporal;
    // esto le pide al AnalysisManager fusionarlos en uno solo al cerrar.
    G4AnalysisManager::Instance()->SetNtupleMerging(true);
}

RunAction::~RunAction() {}

void RunAction::IncrementCaptures()
{
    G4AutoLock lock(&sCaptureMutex);
    ++sTotalCaptures;
}

void RunAction::BeginOfRunAction(const G4Run*)
{
    fTimer.Start();

    if (G4Threading::IsMasterThread()) {
        G4AutoLock lock(&sCaptureMutex);
        sTotalCaptures = 0;
        EventAction::ResetProgress();
    }

    auto analysisManager = G4AnalysisManager::Instance();
    G4cout << "Using " << analysisManager->GetType() << " as analysis backend" << G4endl;

    analysisManager->OpenFile("output.root");

    // ============================================================
    // NTUPLE 0 — ParticulasTransmitidas
    // Partículas registradas al LLEGAR al detector (boundary entry).
    // Misma estructura que ProductosCaptura para comparación directa.
    // ============================================================
    analysisManager->CreateNtuple("ParticulasTransmitidas",
        "Particulas al llegar al detector — pos/energia en la frontera");

    analysisManager->CreateNtupleIColumn("EventID");           // 0
    analysisManager->CreateNtupleSColumn("ParticleType");      // 1
    analysisManager->CreateNtupleDColumn("KineticEnergy_MeV"); // 2
    analysisManager->CreateNtupleDColumn("DirX");              // 3
    analysisManager->CreateNtupleDColumn("DirY");              // 4
    analysisManager->CreateNtupleDColumn("DirZ");              // 5
    analysisManager->CreateNtupleDColumn("PosX_cm");           // 6
    analysisManager->CreateNtupleDColumn("PosY_cm");           // 7
    analysisManager->CreateNtupleDColumn("PosZ_cm");           // 8
    analysisManager->CreateNtupleSColumn("CreatorProcess");    // 9
    analysisManager->CreateNtupleIColumn("TargetZ");           // 10
    analysisManager->CreateNtupleIColumn("TargetA");           // 11
    analysisManager->CreateNtupleSColumn("VertexVolume");      // 12 volumen donde nacio la particula

    analysisManager->FinishNtuple(); // ID = 0

    // ============================================================
    // NTUPLE 1 — ProductosCaptura
    // Partículas registradas al NACER en el grafeno (vértice de creación).
    // Misma estructura que ParticulasTransmitidas para comparación directa.
    // ============================================================
    analysisManager->CreateNtuple("ProductosCaptura",
        "Secundarias al nacer en grafeno — pos/energia en el vertice");

    analysisManager->CreateNtupleIColumn("EventID");           // 0
    analysisManager->CreateNtupleSColumn("ParticleType");      // 1
    analysisManager->CreateNtupleDColumn("KineticEnergy_MeV"); // 2
    analysisManager->CreateNtupleDColumn("DirX");              // 3
    analysisManager->CreateNtupleDColumn("DirY");              // 4
    analysisManager->CreateNtupleDColumn("DirZ");              // 5
    analysisManager->CreateNtupleDColumn("PosX_cm");           // 6
    analysisManager->CreateNtupleDColumn("PosY_cm");           // 7
    analysisManager->CreateNtupleDColumn("PosZ_cm");           // 8
    analysisManager->CreateNtupleSColumn("CreatorProcess");    // 9
    analysisManager->CreateNtupleIColumn("TargetZ");           // 10
    analysisManager->CreateNtupleIColumn("TargetA");           // 11

    analysisManager->FinishNtuple(); // ID = 1

    // ============================================================
    // NTUPLE 2 — RangoIones
    // Desplazamiento total de alfa y Li-7 desde su creación hasta
    // que se detienen. Mide cuánto penetran en el material.
    // ============================================================
    analysisManager->CreateNtuple("RangoIones",
        "Rango total de iones alfa y Li-7 creados en el grafeno");

    analysisManager->CreateNtupleIColumn("EventID");           // 0
    analysisManager->CreateNtupleSColumn("ParticleType");      // 1
    analysisManager->CreateNtupleDColumn("VertexEnergy_MeV"); // 2
    analysisManager->CreateNtupleDColumn("TrackLength_um");   // 3
    analysisManager->CreateNtupleSColumn("CreatorProcess");   // 4

    analysisManager->FinishNtuple(); // ID = 2
}

void RunAction::EndOfRunAction(const G4Run* run)
{
    auto analysisManager = G4AnalysisManager::Instance();
    analysisManager->Write();
    analysisManager->CloseFile();

    fTimer.Stop();

    G4int totalEvents = run->GetNumberOfEvent();
    G4int captures;
    {
        G4AutoLock lock(&sCaptureMutex);
        captures = sTotalCaptures;
    }
    G4double efficiency = (totalEvents > 0)
        ? 100.0 * captures / totalEvents
        : 0.0;

    G4double real = fTimer.GetRealElapsed();
    G4int h = (G4int)(real / 3600);
    G4int m = (G4int)(real / 60) % 60;
    G4int s = (G4int)(real) % 60;

    G4cout << "\n=== SIMULATION STATISTICS ===" << G4endl;
    G4cout << "Neutrons fired:     " << totalEvents << G4endl;
    G4cout << "Captures (B-10):    " << captures << G4endl;
    G4cout << "Capture efficiency: " << efficiency << " %" << G4endl;
    G4cout << "Tiempo total: " << h << "h " << m << "m " << s << "s"
           << "  (" << std::fixed << std::setprecision(1) << real << " s)" << G4endl;
    G4cout << "ROOT file: output.root" << G4endl;
    G4cout << "==============================\n" << G4endl;
}
