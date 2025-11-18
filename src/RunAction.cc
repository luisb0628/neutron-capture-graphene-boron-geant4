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
  auto analysisManager = G4AnalysisManager::Instance();
  G4cout << "Usando " << analysisManager->GetType() << " como backend de análisis" << G4endl;

  // --- Archivo ROOT principal ---
  analysisManager->OpenFile("output.root");

  // --- Histogramas ---
  analysisManager->CreateH1("GamaEnergy_Transmited", "Espectro de energia de gammas en el transmited",
                            1000, 0., 10.*MeV); // ID 0
  analysisManager->CreateH1("AlphaEnergy", "Energia de particulas alfa", 
                            1000, 0., 10.*MeV); // ID 1
  analysisManager->CreateH1("LitioEnergy", "Energia de iones de litio", 
                            1000, 0., 10.*MeV); // ID 2
  analysisManager->CreateH1("GammaEnergy", "Energia de rayos gamma", 
                            1000, 0., 10.*MeV); // ID 3
  analysisManager->CreateH1("AlphaTheta", "Distribucion angular de particulas alfa (theta)",
                            180, 0., 180.); // ID 4 

  // ============================================================
  // Ntuple para partículas transmitidas (neutrones y gammas)
  // ============================================================
  analysisManager->CreateNtuple("GammaNeutronData", "Partículas transmitidas (neutrones y gammas)");
  analysisManager->CreateNtupleIColumn("EventID");           // 0
  analysisManager->CreateNtupleIColumn("TrackID");           // 1
  analysisManager->CreateNtupleIColumn("ParentID");          // 2
  analysisManager->CreateNtupleSColumn("ParticleType");      // 3
  analysisManager->CreateNtupleDColumn("KineticEnergy_MeV"); // 4
  analysisManager->CreateNtupleDColumn("PosX_mm");           // 5
  analysisManager->CreateNtupleDColumn("PosY_mm");           // 6
  analysisManager->CreateNtupleDColumn("PosZ_mm");           // 7
  analysisManager->CreateNtupleDColumn("DirX");              // 8
  analysisManager->CreateNtupleDColumn("DirY");              // 9
  analysisManager->CreateNtupleDColumn("DirZ");              // 10
  analysisManager->CreateNtupleDColumn("Time_ns");           // 11
  analysisManager->CreateNtupleSColumn("VolumeName");        // 12
  analysisManager->CreateNtupleSColumn("CreatorName");       // 13
  analysisManager->FinishNtuple();

  // ============================================================
  // Ntuple para partículas de captura (ya existente)
  // ============================================================
  analysisManager->CreateNtuple("CaptureData", "Datos de particulas de captura");
  analysisManager->CreateNtupleIColumn("EventID");
  analysisManager->CreateNtupleIColumn("TrackID");
  analysisManager->CreateNtupleIColumn("ParentID");
  analysisManager->CreateNtupleSColumn("ParticleType");
  analysisManager->CreateNtupleIColumn("RegionType");
  analysisManager->CreateNtupleDColumn("KineticEnergy_MeV");
  analysisManager->CreateNtupleDColumn("Edep_MeV");
  analysisManager->CreateNtupleDColumn("PosX_mm");
  analysisManager->CreateNtupleDColumn("PosY_mm");
  analysisManager->CreateNtupleDColumn("PosZ_mm");
  analysisManager->CreateNtupleDColumn("DirX");
  analysisManager->CreateNtupleDColumn("DirY");
  analysisManager->CreateNtupleDColumn("DirZ");
  analysisManager->CreateNtupleDColumn("StepLength_mm");
  analysisManager->CreateNtupleDColumn("Time_ns");
  analysisManager->CreateNtupleDColumn("Theta_deg");
  analysisManager->CreateNtupleSColumn("VolumeName");
  analysisManager->CreateNtupleSColumn("CreatorName");  
  analysisManager->CreateNtupleSColumn("ProcessName");
  analysisManager->FinishNtuple();

  // --- Archivo ASCII de salida ---
  outputFile.open("generated_particles.txt");
  if (!outputFile.is_open()) {
      G4Exception("RunAction::BeginOfRunAction", "FileError", FatalException,
                  "No se pudo abrir generated_particles.txt para escritura");
  } else {
      G4cout << "\nArchivo de partículas generado: generated_particles.txt\n" << G4endl;
      outputFile << "# EventID  TrackID  ParentID  Particle  Energy(MeV)  "
                 << "X(mm)  Y(mm)  Z(mm)  DirX  DirY  DirZ  Time(ns)  Process\n";
  }
}

void RunAction::EndOfRunAction(const G4Run* run)
{
  auto analysisManager = G4AnalysisManager::Instance();

  // --- Escribir y cerrar ROOT ---
  analysisManager->Write();
  analysisManager->CloseFile();

  // --- Cerrar archivo ASCII ---
  if (outputFile.is_open()) {
      outputFile.close();
      G4cout << "\nArchivo ASCII guardado: generated_particles.txt" << G4endl;
  }

  // --- Estadísticas ---
  G4cout << "\n=== ESTADÍSTICAS DE LA SIMULACIÓN ===" << G4endl;
  G4cout << "Eventos procesados: " << run->GetNumberOfEvent() << G4endl;
  G4cout << "\nArchivo ROOT guardado: output.root" << G4endl;
  G4cout << "=====================================" << G4endl;
}
