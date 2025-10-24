#include "RunAction.hh"
#include "G4Run.hh"
#include "G4AnalysisManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"

RunAction::RunAction() : G4UserRunAction() {}

RunAction::~RunAction() {}

void RunAction::BeginOfRunAction(const G4Run*)
{
  auto analysisManager = G4AnalysisManager::Instance();
  G4cout << "Usando " << analysisManager->GetType() << " como backend de análisis" << G4endl;

  // Crear archivo ROOT
  analysisManager->OpenFile("output.root");

  // --- Histograma para neutrones transmitidos (llenado por TransmittedSD) ---
  analysisManager->CreateH1("Neutrones", "Espectro de energia de neutrones",
                            200000, 0., .5e7);

  // --- Histogramas para partículas de captura (llenados por CaptureSD) ---
  
  // ID 2: Energía de partículas alfa
  analysisManager->CreateH1("AlphaEnergy", "Energia de particulas alfa", 
                            1000, 0., 10.*MeV);
  
  // ID 3: Energía de iones de litio
  analysisManager->CreateH1("LitioEnergy", "Energia de iones de litio", 
                            1000, 0., 10.*MeV);
  
  // ID 4: Energía de rayos gamma
  analysisManager->CreateH1("GammaEnergy", "Energia de rayos gamma", 
                            1000, 0., 10.*MeV);
  
  // ID 5: Distribución angular de partículas alfa
  analysisManager->CreateH1("AlphaTheta", "Distribucion angular de particulas alfa (theta)",
                            180, 0., 180.);
  
  // ID 6: Tiempo de detección
  analysisManager->CreateH1("DetectionTime", "Tiempo de deteccion de particulas",
                            1000, 0., 100.*ns);

  // --- Ntuple para neutrones transmitidos (llenado por TransmittedSD) ---
  analysisManager->CreateNtuple("NeutronData", "Datos de neutrones transmitidos");
  analysisManager->CreateNtupleDColumn("KineticEnergy_eV"); 
  analysisManager->FinishNtuple();

  // --- Ntuple para partículas de captura (llenado por CaptureSD) ---
  analysisManager->CreateNtuple("CaptureData", "Datos de particulas de captura");
  analysisManager->CreateNtupleIColumn("ParticleType"); // 1=litio, 2=alpha, 3=gamma
  analysisManager->CreateNtupleDColumn("KineticEnergy_MeV");
  analysisManager->CreateNtupleDColumn("Edep_MeV");
  analysisManager->CreateNtupleDColumn("PosX_mm");
  analysisManager->CreateNtupleDColumn("PosY_mm");
  analysisManager->CreateNtupleDColumn("PosZ_mm");
  analysisManager->CreateNtupleDColumn("Time_ns");
  analysisManager->CreateNtupleDColumn("Theta_deg");
  analysisManager->CreateNtupleSColumn("VolumeName");
  analysisManager->CreateNtupleSColumn("ProcessName");
  analysisManager->FinishNtuple();
}

void RunAction::EndOfRunAction(const G4Run* run)
{
  auto analysisManager = G4AnalysisManager::Instance();
  
  // Escribir y cerrar archivo
  analysisManager->Write();
  analysisManager->CloseFile();

  // Mostrar estadísticas
  G4cout << "\n=== ESTADÍSTICAS DE LA SIMULACIÓN ===" << G4endl;
  G4cout << "Eventos procesados: " << run->GetNumberOfEvent() << G4endl;
  
  // Mostrar estadísticas de los histogramas de captura
  G4cout << "\n--- Partículas Detectadas ---" << G4endl;
  G4cout << "Partículas Alfa: " << analysisManager->GetH1(2)->entries() << G4endl;
  G4cout << "Iones de Litio: " << analysisManager->GetH1(3)->entries() << G4endl;
  G4cout << "Rayos Gamma: " << analysisManager->GetH1(4)->entries() << G4endl;
  
  G4cout << "\nArchivo ROOT guardado: output.root" << G4endl;
  G4cout << "=====================================" << G4endl;
}