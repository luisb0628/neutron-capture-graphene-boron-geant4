#include "DetectorConstruction.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SDManager.hh"
#include "TransmittedSD.hh"
#include "CaptureSD.hh"
#include "G4SystemOfUnits.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4UserLimits.hh"
#include "G4ProductionCuts.hh"
#include "G4Region.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4Isotope.hh"
#include "G4Element.hh"

DetectorConstruction::DetectorConstruction() : G4VUserDetectorConstruction() {}
DetectorConstruction::~DetectorConstruction() = default;

G4VPhysicalVolume* DetectorConstruction::Construct() {
    auto nist = G4NistManager::Instance();

    // --- Mundo ---
    auto worldMat = nist->FindOrBuildMaterial("G4_Galactic"); // vacío para térmicos
    auto solidWorld = new G4Box("World", 3*cm, 3*cm, 2*cm);
    auto logicWorld = new G4LogicalVolume(solidWorld, worldMat, "World");
    auto physWorld  = new G4PVPlacement(0, G4ThreeVector(), logicWorld, "World", 0, false, 0);

    // --- Isótopos de Boro enriquecido ---
    auto B10 = new G4Isotope("B10", 5, 10, 10.0*g/mole);
    auto B11 = new G4Isotope("B11", 5, 11, 11.0*g/mole);
    auto elB_enriched = new G4Element("BoronEnriched", "B_enr", 2);
    elB_enriched->AddIsotope(B10, 90.*perCent);  // 90% B10
    elB_enriched->AddIsotope(B11, 10.*perCent);  // 10% B11

    // --- Film de grafeno dopado con Boro ---
    G4double grapheneThickness = 100 * nm;
    G4double grapheneHalfZ = grapheneThickness / 2.0;

    auto graphene = new G4Material("graphene", 2.2*g/cm3, 2);
    graphene->AddElement(nist->FindOrBuildElement("C"), 0.85);
    graphene->AddElement(elB_enriched, 0.15);

    auto solidgraphene = new G4Box("graphene", 1*cm, 1*cm, grapheneHalfZ); // 2cmx2cm total
    auto logicgraphene = new G4LogicalVolume(solidgraphene, graphene, "graphene");
    new G4PVPlacement(0, G4ThreeVector(0,0,0), logicgraphene, "graphene", logicWorld, false, 0);

    // --- Film Kapton (pegado al grafeno) ---
    G4double kaptonThickness = 50*um;
    G4double kaptonHalfZ = kaptonThickness/2.0;

    G4Material* kaptonMat = nist->FindOrBuildMaterial("G4_KAPTON");

    // Colocamos el kapton justo detrás del grafeno (sin aire entre ellos)
    G4double kaptonZ = grapheneHalfZ + kaptonHalfZ;
    auto solidkapton = new G4Box("kapton", 1*cm, 1*cm, kaptonHalfZ);
    auto logickapton = new G4LogicalVolume(solidkapton, kaptonMat, "kapton");
    new G4PVPlacement(0, G4ThreeVector(0,0,kaptonZ), logickapton, "kapton", logicWorld, false, 0);

    // --- Detector plano (neutrones transmitidos) ---
    G4double detThickness = 0.5*mm;
    G4double detHalfZ = detThickness / 2.0;
    G4double detZ = kaptonZ + kaptonHalfZ + detHalfZ;
    auto detMat = nist->FindOrBuildMaterial("G4_AIR");
    auto solidDet = new G4Box("Detector", 1*cm, 1*cm, detHalfZ);
    auto logicDet = new G4LogicalVolume(solidDet, detMat, "Detector");
    new G4PVPlacement(0, G4ThreeVector(0,0,detZ), logicDet, "Detector", logicWorld, false, 0);

    // --- Detector de captura (productos de reacción) ---
    //G4double capThickness = 0.5*mm;
    //G4double capHalfZ = capThickness / 2.0;
    //G4double capZ = detZ + detHalfZ + capHalfZ;
    //auto capMat = nist->FindOrBuildMaterial("G4_Si");
    //auto solidCap = new G4Box("Capture", 1*cm, 1*cm, capHalfZ);
    //auto logicCap = new G4LogicalVolume(solidCap, capMat, "Capture");
    //new G4PVPlacement(0, G4ThreeVector(0,0,capZ), logicCap, "Capture", logicWorld, false, 0);

    // --- Límites de paso ---
    logicWorld->SetUserLimits(new G4UserLimits(1.0*mm));      // mundo
    logicgraphene->SetUserLimits(new G4UserLimits(0.01*mm));  // precisión en film
    logickapton->SetUserLimits(new G4UserLimits(0.01*mm));
    logicDet->SetUserLimits(new G4UserLimits(0.01*mm));
    //logicCap->SetUserLimits(new G4UserLimits(0.01*mm));

    logicWorld->SetVisAttributes(G4VisAttributes::GetInvisible());

    // --- Cortes de producción ---
    auto cuts = new G4ProductionCuts();
    cuts->SetProductionCut(0.01*mm, G4ProductionCuts::GetIndex("neutron"));
    cuts->SetProductionCut(0.01*mm, G4ProductionCuts::GetIndex("gamma"));
    cuts->SetProductionCut(0.01*mm, G4ProductionCuts::GetIndex("e-"));

    // Asignar región
    auto region = new G4Region("DetectorRegion");
    region->AddRootLogicalVolume(logicgraphene);
    region->AddRootLogicalVolume(logickapton);
    region->AddRootLogicalVolume(logicDet);
    //region->AddRootLogicalVolume(logicCap);
    region->SetProductionCuts(cuts);

    // --- Atributos visuales ---
    auto visGraphene = new G4VisAttributes(G4Colour(0.0, 0.0, 1.0, 0.7)); // Azul translúcido
    visGraphene->SetForceSolid(true);
    logicgraphene->SetVisAttributes(visGraphene);

    auto visKapton = new G4VisAttributes(G4Colour(1.0, 0.7, 0.3, 0.4)); // Naranja translúcido
    visKapton->SetForceSolid(true);
    logickapton->SetVisAttributes(visKapton);

    auto visDetector = new G4VisAttributes(G4Colour(0.0, 1.0, 0.0, 0.2)); // Verde translúcido
    visDetector->SetForceSolid(true);
    logicDet->SetVisAttributes(visDetector);

    //auto visCapture = new G4VisAttributes(G4Colour(1.0, 0.0, 0.0, 0.2)); // Rojo translúcido
    //visCapture->SetForceSolid(true);
    //logicCap->SetVisAttributes(visCapture);

    return physWorld;
}

//void DetectorConstruction::ConstructSDandField() {
//    G4SDManager* sdman = G4SDManager::GetSDMpointer();

    // --- Detector transmitidos ---
//    auto transmittedSD = new TransmittedSD("TransmittedSD");
//    sdman->AddNewDetector(transmittedSD);
//    auto lvDet = G4LogicalVolumeStore::GetInstance()->GetVolume("Detector");
//    if (lvDet) lvDet->SetSensitiveDetector(transmittedSD);

    // --- Detector captura ---
    //auto captureSD = new CaptureSD("CaptureSD", "CaptureHitsCollection");
    //sdman->AddNewDetector(captureSD);

    //auto lvCap = G4LogicalVolumeStore::GetInstance()->GetVolume("Capture");
    //if (lvCap) lvCap->SetSensitiveDetector(captureSD);

//    auto lvGraphene = G4LogicalVolumeStore::GetInstance()->GetVolume("graphene");
//    if (lvGraphene) lvGraphene->SetSensitiveDetector(captureSD);
//}

void DetectorConstruction::ConstructSDandField() {
    G4SDManager* sdman = G4SDManager::GetSDMpointer();

    // --- Crear el Sensitive Detector para el film de grafeno ---
    auto grapheneSD = new CaptureSD("GrapheneSD", "GrapheneHitsCollection");
    sdman->AddNewDetector(grapheneSD);

    // --- Asignar el SD al volumen del film de grafeno ---
    auto lvGraphene = G4LogicalVolumeStore::GetInstance()->GetVolume("graphene");
    if (lvGraphene) {
        lvGraphene->SetSensitiveDetector(grapheneSD);
    } else {
        G4Exception("DetectorConstruction::ConstructSDandField", "SD001", JustWarning,
                    "Logical volume 'graphene' not found to attach sensitive detector.");
    }
}