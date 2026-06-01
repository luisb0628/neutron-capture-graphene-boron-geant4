#include "RunAction.hh"
#include "G4Run.hh"
#include "G4AnalysisManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"
#include "G4ios.hh"

RunAction::RunAction() : G4UserRunAction() {}

RunAction::~RunAction() {}

void RunAction::BeginOfRunAction(const G4Run*)
{
    fNeutronsCaptured = 0;

    auto analysisManager = G4AnalysisManager::Instance();
    G4cout << "Using " << analysisManager->GetType() << " as analysis backend" << G4endl;

    analysisManager->OpenFile("output.root");

    // ============================================================
    // NTUPLE 0 — ParticulasDetector
    // Partículas que llegan al detector sensible (detrás del grafeno).
    // VertexVolume indica el origen: "graphene" (captura B-10) o "kapton"
    // (interacciones del Kapton que generaron la partícula).
    // ============================================================
    analysisManager->CreateNtuple("ParticulasDetector",
        "Particulas que alcanzan el detector; VertexVolume indica su origen");

    analysisManager->CreateNtupleIColumn("EventID");           // 0
    analysisManager->CreateNtupleSColumn("ParticleType");      // 1
    analysisManager->CreateNtupleDColumn("KineticEnergy_keV"); // 2
    analysisManager->CreateNtupleDColumn("DirX");              // 3
    analysisManager->CreateNtupleDColumn("DirY");              // 4
    analysisManager->CreateNtupleDColumn("DirZ");              // 5
    analysisManager->CreateNtupleSColumn("VertexVolume");      // 6
    analysisManager->CreateNtupleSColumn("CreatorProcess");    // 7
    analysisManager->CreateNtupleIColumn("TargetZ");           // 8
    analysisManager->CreateNtupleIColumn("TargetA");           // 9
    analysisManager->CreateNtupleDColumn("PosX_cm");           // 10
    analysisManager->CreateNtupleDColumn("PosY_cm");           // 11
    analysisManager->CreateNtupleDColumn("PosZ_cm");           // 12

    analysisManager->FinishNtuple(); // ID = 0

    // ============================================================
    // NTUPLE 1 — ProductosCaptura
    // Partículas creadas por captura neutrónica en el grafeno (B-10).
    // Una entrada por cada secundaria nacida en el grafeno.
    // ============================================================
    analysisManager->CreateNtuple("ProductosCaptura",
        "Secundarias nacidas en grafeno por captura de neutron en B-10");

    analysisManager->CreateNtupleIColumn("EventID");           // 0
    analysisManager->CreateNtupleSColumn("ParticleType");      // 1
    analysisManager->CreateNtupleDColumn("KineticEnergy_MeV"); // 2
    analysisManager->CreateNtupleDColumn("Edep_MeV");          // 3  deposición en primer paso
    analysisManager->CreateNtupleDColumn("DirX");              // 4
    analysisManager->CreateNtupleDColumn("DirY");              // 5
    analysisManager->CreateNtupleDColumn("DirZ");              // 6
    analysisManager->CreateNtupleDColumn("StepLength_um");     // 7  longitud del primer paso
    analysisManager->CreateNtupleSColumn("CreatorProcess");    // 8
    analysisManager->CreateNtupleIColumn("TargetZ");           // 9
    analysisManager->CreateNtupleIColumn("TargetA");           // 10

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

    // ============================================================
    // Archivo ASCII — ProductosCaptura (nacidos en grafeno)
    // ============================================================
    outputFile.open("generated_particles.txt");
    if (!outputFile.is_open()) {
        G4Exception("RunAction::BeginOfRunAction", "FileError", FatalException,
                    "Could not open generated_particles.txt for writing");
    } else {
        G4cout << "\nASCII output: generated_particles.txt\n" << G4endl;
        outputFile << "# EventID Particle KinEnergy_MeV Edep_MeV"
                   << " DirX DirY DirZ StepLen_um CreatorProcess TargetZ TargetA\n";
    }

    // ============================================================
    // Archivo ASCII — ParticulasTransmitidas (todo lo que llega al detector)
    // ============================================================
    transmittedFile.open("transmitted_particles.txt");
    if (!transmittedFile.is_open()) {
        G4Exception("RunAction::BeginOfRunAction", "FileError", FatalException,
                    "Could not open transmitted_particles.txt for writing");
    } else {
        G4cout << "ASCII output: transmitted_particles.txt\n" << G4endl;
        transmittedFile << "# EventID Particle KinEnergy_MeV"
                        << " DirX DirY DirZ VertexVolume CreatorProcess TargetZ TargetA\n";
    }
}

void RunAction::EndOfRunAction(const G4Run* run)
{
    auto analysisManager = G4AnalysisManager::Instance();
    analysisManager->Write();
    analysisManager->CloseFile();

    if (outputFile.is_open()) {
        outputFile.close();
        G4cout << "ASCII file saved: generated_particles.txt" << G4endl;
    }
    if (transmittedFile.is_open()) {
        transmittedFile.close();
        G4cout << "ASCII file saved: transmitted_particles.txt" << G4endl;
    }

    G4int totalEvents = run->GetNumberOfEvent();
    G4double efficiency = (totalEvents > 0)
        ? 100.0 * fNeutronsCaptured / totalEvents
        : 0.0;

    G4cout << "\n=== SIMULATION STATISTICS ===" << G4endl;
    G4cout << "Neutrons fired:    " << totalEvents << G4endl;
    G4cout << "Captures (B-10):   " << fNeutronsCaptured << G4endl;
    G4cout << "Capture efficiency: " << efficiency << " %" << G4endl;
    G4cout << "ROOT file: output.root" << G4endl;
    G4cout << "==============================\n" << G4endl;
}
