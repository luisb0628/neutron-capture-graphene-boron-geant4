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
    auto worldMat = nist->FindOrBuildMaterial("G4_AIR");
    auto solidWorld = new G4Box("World", 10*cm, 10*cm, 10*cm);
    auto logicWorld = new G4LogicalVolume(solidWorld, worldMat, "World");
    auto physWorld  = new G4PVPlacement(0, G4ThreeVector(), logicWorld, "World", 0, false, 0);

    // --- Isótopos de Boro enriquecido ---
    auto B10 = new G4Isotope("B10", 5, 10, 10.0*g/mole);
    auto B11 = new G4Isotope("B11", 5, 11, 11.0*g/mole);
    auto elB_enriched = new G4Element("BoronEnriched", "B_enr", 2);
    elB_enriched->AddIsotope(B10, 100.*perCent);
    elB_enriched->AddIsotope(B11, 0.*perCent);

    // --- Film de grafeno dopado con Boro ---
    G4double grapheneThickness = 10 * um;
    G4double grapheneHalfZ = grapheneThickness / 2.0;

    auto graphene = new G4Material("graphene", 2.2*g/cm3, 2);
    graphene->AddElement(nist->FindOrBuildElement("C"), 0.85);
    graphene->AddElement(elB_enriched, 0.15);

    auto solidgraphene = new G4Box("graphene", 1*cm, 1*cm, grapheneHalfZ);
    auto logicgraphene = new G4LogicalVolume(solidgraphene, graphene, "graphene");
    new G4PVPlacement(0, G4ThreeVector(0,0,0), logicgraphene, "graphene", logicWorld, false, 0);

    // --- Film Kapton (detrás del grafeno) ---
    G4double kaptonThickness = 50*um;
    G4double kaptonHalfZ = kaptonThickness/2.0;
    G4Material* kaptonMat = nist->FindOrBuildMaterial("G4_KAPTON");

    G4double kaptonZ = grapheneHalfZ + kaptonHalfZ;
    auto solidkapton = new G4Box("kapton", 1.5*cm, 1.5*cm, kaptonHalfZ);
    auto logickapton = new G4LogicalVolume(solidkapton, kaptonMat, "kapton");
    new G4PVPlacement(0, G4ThreeVector(0,0,kaptonZ), logickapton, "kapton", logicWorld, false, 0);

    // --- Detector plano (neutrones transmitidos) ---
    G4double detThickness = 0.5*nm;
    G4double detHalfZ = detThickness / 2.0;
    G4double detZ = kaptonZ + kaptonHalfZ + detHalfZ;
    auto detMat = nist->FindOrBuildMaterial("G4_AIR");
    auto solidDet = new G4Box("Detector", 1*cm, 1*cm, detHalfZ);
    auto logicDet = new G4LogicalVolume(solidDet, detMat, "Detector");
    new G4PVPlacement(0, G4ThreeVector(0,0,detZ), logicDet, "Detector", logicWorld, false, 0);

    // --- Límites de paso ---
    logicWorld->SetUserLimits(new G4UserLimits(1.0*mm));
    logicgraphene->SetUserLimits(new G4UserLimits(0.01*mm));
    logickapton->SetUserLimits(new G4UserLimits(0.01*mm));
    logicDet->SetUserLimits(new G4UserLimits(0.01*mm));

    logicWorld->SetVisAttributes(G4VisAttributes::GetInvisible());

    // --- Cortes de producción ---
    auto cuts = new G4ProductionCuts();
    cuts->SetProductionCut(0.01*mm, G4ProductionCuts::GetIndex("neutron"));
    cuts->SetProductionCut(0.01*mm, G4ProductionCuts::GetIndex("gamma"));
    cuts->SetProductionCut(0.01*mm, G4ProductionCuts::GetIndex("e-"));

    auto region = new G4Region("DetectorRegion");
    region->AddRootLogicalVolume(logicgraphene);
    region->AddRootLogicalVolume(logickapton);
    region->AddRootLogicalVolume(logicDet);
    region->SetProductionCuts(cuts);

    // --- Atributos visuales ---
    auto visGraphene = new G4VisAttributes(G4Colour(0.0, 0.0, 1.0, 0.7)); // Azul
    visGraphene->SetForceSolid(true);
    logicgraphene->SetVisAttributes(visGraphene);

    auto visKapton = new G4VisAttributes(G4Colour(1.0, 0.7, 0.3, 0.4)); // Naranja
    visKapton->SetForceSolid(true);
    logickapton->SetVisAttributes(visKapton);

    auto visDetector = new G4VisAttributes(G4Colour(0.0, 1.0, 0.0, 0.2)); // Verde
    visDetector->SetForceSolid(true);
    logicDet->SetVisAttributes(visDetector);

    return physWorld;
}

void DetectorConstruction::ConstructSDandField() {
    G4SDManager* sdman = G4SDManager::GetSDMpointer();

    // --- Sensitive detector para el grafeno (capturas de neutrones) ---
    auto grapheneSD = new CaptureSD("GrapheneSD", "GrapheneHitsCollection");
    sdman->AddNewDetector(grapheneSD);

    auto lvGraphene = G4LogicalVolumeStore::GetInstance()->GetVolume("graphene");
    if (lvGraphene) {
        lvGraphene->SetSensitiveDetector(grapheneSD);
    } else {
        G4Exception("DetectorConstruction::ConstructSDandField", "SD001", JustWarning,
                    "Logical volume 'graphene' not found to attach CaptureSD.");
    }

    // --- Sensitive detector para neutrones transmitidos ---
    auto transmittedSD = new TransmittedSD("TransmittedSD");
    sdman->AddNewDetector(transmittedSD);

    auto lvDet = G4LogicalVolumeStore::GetInstance()->GetVolume("Detector");
    if (lvDet) {
        lvDet->SetSensitiveDetector(transmittedSD);
    } else {
        G4Exception("DetectorConstruction::ConstructSDandField", "SD002", JustWarning,
                    "Logical volume 'Detector' not found to attach TransmittedSD.");
    }
}
