#include "DetectorConstruction.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SDManager.hh"
#include "TransmittedSD.hh"
#include "G4SystemOfUnits.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4UserLimits.hh"
#include "G4ProductionCuts.hh"
#include "G4Region.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "CaptureSD.hh"

DetectorConstruction::DetectorConstruction() : G4VUserDetectorConstruction() {}
DetectorConstruction::~DetectorConstruction() = default;

G4VPhysicalVolume* DetectorConstruction::Construct() {
    auto nist = G4NistManager::Instance();

    // --- Mundo ---
    auto worldMat = nist->FindOrBuildMaterial("G4_AIR");
    auto solidWorld = new G4Box("World", 2*cm, 2*cm, 1*cm);
    auto logicWorld = new G4LogicalVolume(solidWorld, worldMat, "World");
    auto physWorld  = new G4PVPlacement(0, G4ThreeVector(), logicWorld, "World", 0, false, 0);

    // --- Film de grafeno ---
    fgrapheneThickness = 100 * nm;
    G4double halfgrapheneZ = fgrapheneThickness / 2.0;

    auto graphene = new G4Material("graphene", 2.2*g/cm3, 2);
    graphene->AddElement(nist->FindOrBuildElement("C"), 0.85);
    graphene->AddElement(nist->FindOrBuildElement("B"), 0.15);

    auto solidgraphene = new G4Box("graphene", 0.5*cm, 0.5*cm, halfgrapheneZ);
    auto logicgraphene = new G4LogicalVolume(solidgraphene, graphene, "graphene");
    new G4PVPlacement(0, G4ThreeVector(0,0,0), logicgraphene, "graphene", logicWorld, false, 0);

    // --- Film kapton ---
    G4Material* kaptonMat = nist->FindOrBuildMaterial("G4_KAPTON");
    auto solidkapton = new G4Box("kapton", 1*cm, 1*cm, 50*um);
    auto logickapton = new G4LogicalVolume(solidkapton, kaptonMat, "kapton");
    new G4PVPlacement(0, G4ThreeVector(0,0,0.1*cm), logickapton, "kapton", logicWorld, false, 0);

    // --- Detector plano (para partículas transmitidas) ---
    auto solidDet = new G4Box("Detector", 1.5*cm, 1.5*cm, 0.5*mm);
    auto detMat = nist->FindOrBuildMaterial("G4_AIR");
    auto logicDet = new G4LogicalVolume(solidDet, detMat, "Detector");
    G4double zPos = halfgrapheneZ + 0.1*cm + 0.5*mm;
    new G4PVPlacement(0, G4ThreeVector(0,0,zPos), logicDet, "Detector", logicWorld, false, 0);

    // --- Detector de captura (para partículas de la reacción) ---
    auto solidCap = new G4Box("Capture", 2*cm, 2*cm, 0.5*mm);
    auto capMat = nist->FindOrBuildMaterial("G4_Si"); // Usar silicio para mejor detección
    auto logicCap = new G4LogicalVolume(solidCap, capMat, "Capture");
    // Colocar el detector de captura en una posición diferente
    new G4PVPlacement(0, G4ThreeVector(0,0,zPos+1*mm), logicCap, "Capture", logicWorld, false, 0);

    // --- Límites de paso para mayor resolución ---
    G4double maxStep = 0.01*mm;
    auto userLimits = new G4UserLimits(maxStep);
    logicWorld->SetUserLimits(userLimits);
    logicgraphene->SetUserLimits(userLimits);
    logickapton->SetUserLimits(userLimits);
    logicDet->SetUserLimits(userLimits);
    logicCap->SetUserLimits(userLimits);
    
    logicWorld->SetVisAttributes(G4VisAttributes::GetInvisible());

    // --- Cortes de producción ---
    auto cuts = new G4ProductionCuts();
    cuts->SetProductionCut(0.001*mm, "neutron");   // permitir neutrones térmicos
    cuts->SetProductionCut(0.001*mm, "gamma");
    cuts->SetProductionCut(0.001*mm, "e-");

    // Asignar cortes a una región
    auto region = new G4Region("DetectorRegion");
    region->AddRootLogicalVolume(logicgraphene);
    region->AddRootLogicalVolume(logickapton);
    region->AddRootLogicalVolume(logicDet);
    region->AddRootLogicalVolume(logicCap);
    region->SetProductionCuts(cuts);

    // --- Atributos visuales ---
    auto visKapton = new G4VisAttributes(G4Colour(1.0, 0.7, 0.3, 0.4)); // Naranja translúcido
    visKapton->SetForceSolid(true);
    logickapton->SetVisAttributes(visKapton);

    auto visGraphene = new G4VisAttributes(G4Colour(0.0, 0.0, 1.0, 0.7)); // Azul translúcido
    visGraphene->SetForceSolid(true);
    logicgraphene->SetVisAttributes(visGraphene);

    auto visDetector = new G4VisAttributes(G4Colour(0.0, 1.0, 0.0, 0.2)); // Verde para detector transmitido
    visDetector->SetForceSolid(true);
    logicDet->SetVisAttributes(visDetector);

    auto visCapture = new G4VisAttributes(G4Colour(1.0, 0.0, 0.0, 0.2)); // Rojo para detector de captura
    visCapture->SetForceSolid(true);
    logicCap->SetVisAttributes(visCapture);

    return physWorld;
}

void DetectorConstruction::ConstructSDandField() {
    G4SDManager* sdman = G4SDManager::GetSDMpointer();
    
    // --- Detector para partículas transmitidas ---
    auto transmittedSD = new TransmittedSD("TransmittedSD");
    sdman->AddNewDetector(transmittedSD);

    auto lvDet = G4LogicalVolumeStore::GetInstance()->GetVolume("Detector");
    if (lvDet) {
        lvDet->SetSensitiveDetector(transmittedSD);
    } else {
        G4Exception("DetectorConstruction::ConstructSDandField", "SD001", JustWarning,
                    "Logical volume 'Detector' not found to attach sensitive detector.");
    }

    // --- Detector para partículas de captura ---
    auto captureSD = new CaptureSD("CaptureSD", "CaptureHitsCollection");
    sdman->AddNewDetector(captureSD);

    auto lvCap = G4LogicalVolumeStore::GetInstance()->GetVolume("Capture");
    if (lvCap) {
        lvCap->SetSensitiveDetector(captureSD);
    } else {
        G4Exception("DetectorConstruction::ConstructSDandField", "SD002", JustWarning,
                    "Logical volume 'Capture' not found to attach sensitive detector.");
    }

    // También podrías querer hacer sensible el volumen de grafeno para capturar
    // las partículas generadas directamente en él
    auto lvGraphene = G4LogicalVolumeStore::GetInstance()->GetVolume("graphene");
    if (lvGraphene) {
        lvGraphene->SetSensitiveDetector(captureSD);
    }
}