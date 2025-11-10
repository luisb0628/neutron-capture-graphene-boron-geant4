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
  analysisManager->CreateH1("Neutrones", "Espectro de energia de neutrones",
                            200000, 0., .5e7); // ID 0
  analysisManager->CreateH1("AlphaEnergy", "Energia de particulas alfa", 
                            1000, 0., 10.*MeV); // ID 1
  analysisManager->CreateH1("LitioEnergy", "Energia de iones de litio", 
                            1000, 0., 10.*MeV); // ID 2
  analysisManager->CreateH1("GammaEnergy", "Energia de rayos gamma", 
                            1000, 0., 10.*MeV); // ID 3
  analysisManager->CreateH1("AlphaTheta", "Distribucion angular de particulas alfa (theta)",
                            180, 0., 180.); // ID 4 ✅ (con este trabajará CaptureSD)

  // --- Ntuple para neutrones transmitidos ---
  analysisManager->CreateNtuple("NeutronData", "Datos de neutrones transmitidos");
  analysisManager->CreateNtupleDColumn("KineticEnergy_eV"); 
  analysisManager->FinishNtuple();

  // --- Ntuple para partículas de captura ---
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
  G4cout << "\n--- Partículas Detectadas ---" << G4endl;
  G4cout << "Partículas Alfa: " << analysisManager->GetH1(1)->entries() << G4endl;
  G4cout << "Iones de Litio: "  << analysisManager->GetH1(2)->entries() << G4endl;
  G4cout << "Rayos Gamma: "     << analysisManager->GetH1(3)->entries() << G4endl;
  G4cout << "Distribución angular Alfa (theta): "
         << analysisManager->GetH1(4)->entries() << " entradas" << G4endl;

  G4cout << "\nArchivo ROOT guardado: output.root" << G4endl;
  G4cout << "=====================================" << G4endl;
}
