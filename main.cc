#include "G4RunManager.hh"
#include "G4RunManagerFactory.hh"
#include "G4Threading.hh"
#include "G4UImanager.hh"
#include "QGSP_BERT_HP.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"
#include "Randomize.hh"
#include "TROOT.h"

#include "ActionInitialization.hh"
#include "DetectorConstruction.hh"

int main(int argc, char** argv) {
    // Establecer semilla aleatoria para reproducibilidad
    G4Random::setTheSeed(123456789);

    // PrimaryGeneratorAction lee AmBePhaseSpace.root (TFile/TTree) desde el
    // constructor de cada hilo worker; sin esto, aperturas de ROOT concurrentes
    // entre hilos no son seguras.
    ROOT::EnableThreadSafety();

    try {
        // Determinar modo de ejecución
        G4UIExecutive* ui = nullptr;
        if (argc == 1) {
            ui = new G4UIExecutive(argc, argv);
        }

        // Crear el Run Manager multihilo (usa todos los núcleos disponibles;
        // cae a modo serial automáticamente si Geant4 no se compiló con MT)
        G4RunManager* runManager =
            G4RunManagerFactory::CreateRunManager(G4RunManagerType::MT, false);
        runManager->SetNumberOfThreads(G4Threading::G4GetNumberOfCores());

        // Construcción del detector
        runManager->SetUserInitialization(new DetectorConstruction());

        // Configurar física - QGSP_BERT_HP es ideal para neutrones
        auto physicsList = new QGSP_BERT_HP();
        physicsList->SetVerboseLevel(0);  // Silenciar output de física
        runManager->SetUserInitialization(physicsList);

        // Inicialización de acciones de usuario
        runManager->SetUserInitialization(new ActionInitialization());

        // NOTA 2026-10: NO se llama a runManager->Initialize() aqui. En modo
        // MT eso crea y arranca los hilos trabajadores de inmediato -- cada
        // uno construye su PrimaryGeneratorAction (y por lo tanto carga el
        // .root de phase space) en ESE momento, antes de que la macro llegue
        // a ejecutar /generator/phaseSpaceFile mas abajo. El comando se
        // aplicaba, pero demasiado tarde (los hilos ya habian cargado el
        // archivo por defecto). El propio macro ya trae su /run/initialize
        // (ver run.mac/run_cs137.mac etc.), que ocurre DESPUES de
        // /generator/phaseSpaceFile -- igual que en el proyecto hermano
        // neutron-thermalization-geant4, que nunca llamo Initialize() aqui.

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
            UImanager->ApplyCommand("/control/execute ../macros/vis1.mac");
            
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