#include "G4RunManager.hh"
#include "G4UImanager.hh"
#include "QGSP_BERT_HP.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"
#include "G4ScoringManager.hh"
#include "Randomize.hh"  // Importante para semillas aleatorias

#include "ActionInitialization.hh"
#include "DetectorConstruction.hh"

int main(int argc, char** argv) {
    // Establecer semilla aleatoria para reproducibilidad
    G4Random::setTheSeed(123456789);
    
    try {
        // Determinar modo de ejecución
        G4UIExecutive* ui = nullptr;
        if (argc == 1) {
            ui = new G4UIExecutive(argc, argv);
        }

        // Crear Run Manager
        auto* runManager = new G4RunManager();

        // Activar sistema de scoring (útil para análisis)
        G4ScoringManager::GetScoringManager();

        // Construcción del detector
        runManager->SetUserInitialization(new DetectorConstruction());

        // Configurar física - QGSP_BERT_HP es ideal para neutrones
        auto physicsList = new QGSP_BERT_HP();
        physicsList->SetVerboseLevel(0);  // Silenciar output de física
        runManager->SetUserInitialization(physicsList);

        // Inicialización de acciones de usuario
        runManager->SetUserInitialization(new ActionInitialization());

        // Inicializar el run manager
        runManager->Initialize();

        // Sistema de visualización
        auto* visManager = new G4VisExecutive();
        visManager->SetVerboseLevel("errors");  // Solo mostrar errores
        visManager->Initialize();

        // Gestor de interfaz de usuario
        G4UImanager* UImanager = G4UImanager::GetUIpointer();

        // Configuración básica de verbosidad
        UImanager->ApplyCommand("/control/verbose 1");
        UImanager->ApplyCommand("/run/verbose 0");
        UImanager->ApplyCommand("/event/verbose 0");
        UImanager->ApplyCommand("/tracking/verbose 0");

        if (!ui) {
            // Modo batch: ejecutar macro proporcionada
            G4String command = "/control/execute ";
            G4String fileName = argv[1];
            UImanager->ApplyCommand(command + fileName);
        } else {
            // Modo interactivo
            G4cout << G4endl;
            G4cout << "==================================================" << G4endl;
            G4cout << "    Simulación de Captura de Neutrones Térmicos" << G4endl;
            G4cout << "    en Película de Grafeno Dopado con Boro" << G4endl;
            G4cout << "==================================================" << G4endl;
            G4cout << "Partículas detectadas:" << G4endl;
            G4cout << " • Iones de Litio (Li-6, Li-7)" << G4endl;
            G4cout << " • Partículas Alfa (He-4)" << G4endl;
            G4cout << " • Fotones Gamma" << G4endl;
            G4cout << "==================================================" << G4endl;
            G4cout << "Usando vis1.mac para configuración..." << G4endl;
            
            // Ejecutar tu macro de visualización
            UImanager->ApplyCommand("/control/execute vis1.mac");
            
            // Iniciar sesión interactiva
            ui->SessionStart();
            delete ui;
        }

        // Limpieza de memoria
        delete visManager;
        delete runManager;

    } catch (const std::exception& e) {
        G4cerr << "Excepción: " << e.what() << G4endl;
        return 1;
    } catch (...) {
        G4cerr << "Excepción desconocida!" << G4endl;
        return 1;
    }

    return 0;
}