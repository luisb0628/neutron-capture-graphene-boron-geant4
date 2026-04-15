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
#include "G4UserLimits.hh"
#include "G4ProductionCuts.hh"
#include "G4Region.hh"
#include "G4RegionStore.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4Isotope.hh"
#include "G4Element.hh"
#include "G4GenericMessenger.hh"
#include "G4RunManager.hh"
#include <cmath>
#include <string>

DetectorConstruction::DetectorConstruction()
    : G4VUserDetectorConstruction()
{
    DefineMaterials();   // crear materiales UNA sola vez
    DefineCommands();
}

DetectorConstruction::~DetectorConstruction()
{
    delete fMessenger;
}

// ======================================================
// DefineMaterials — se llama solo desde el constructor.
// Los G4Material, G4Element y G4Isotope se registran en
// tablas globales y NO deben crearse más de una vez.
// ======================================================
void DetectorConstruction::DefineMaterials()
{
    auto B10 = new G4Isotope("B10", 5, 10, 10.0*g/mole);
    auto B11 = new G4Isotope("B11", 5, 11, 11.0*g/mole);
    fElB_enriched = new G4Element("BoronEnriched", "B_enr", 2);
    fElB_enriched->AddIsotope(B10, 100.*perCent);
    fElB_enriched->AddIsotope(B11,   0.*perCent);

    BuildGrapheneMaterial();   // crea el material inicial con fBoronFraction
}

// Crea (o reutiliza si ya existe) el material de grafeno con la fracción de boro actual.
// Cada fracción distinta genera un material con nombre único en la tabla global.
void DetectorConstruction::BuildGrapheneMaterial()
{
    auto* nist = G4NistManager::Instance();

    // Nombre único por fracción: e.g. "graphene_B500" = 5.00 % boro
    G4int fractionPPM = G4int(std::round(fBoronFraction * 1e6));
    G4String matName  = "graphene_B" + std::to_string(fractionPPM);

    // Reutilizar si ya fue creado en una corrida anterior
    fGrapheneMat = G4Material::GetMaterial(matName, /*warn=*/false);
    if (!fGrapheneMat) {
        fGrapheneMat = new G4Material(matName, 2.2*g/cm3, 2);
        fGrapheneMat->AddElement(nist->FindOrBuildElement("C"), 1.0 - fBoronFraction);
        fGrapheneMat->AddElement(fElB_enriched,                 fBoronFraction);
    }
}

// ======================================================
// Setters — actualizan el espesor y reinicializan
// ======================================================
void DetectorConstruction::SetGrapheneThickness(G4double t)
{
    fGrapheneThickness = t;
    // Solo reinicializar si la geometría ya fue construida al menos una vez.
    // Si el comando llega antes de /run/initialize, solo guardamos el valor.
    if (fGeometryBuilt)
        G4RunManager::GetRunManager()->ReinitializeGeometry();
}

void DetectorConstruction::SetKaptonThickness(G4double t)
{
    fKaptonThickness = t;
    if (fGeometryBuilt)
        G4RunManager::GetRunManager()->ReinitializeGeometry();
}

void DetectorConstruction::SetBoronFraction(G4double f)
{
    fBoronFraction = f;
    BuildGrapheneMaterial();   // crear/recuperar material con nueva fracción
    if (fGeometryBuilt) {
        // Actualizar el LV inmediatamente para que la próxima reinit use el material correcto
        if (fLogicGraphene) fLogicGraphene->SetMaterial(fGrapheneMat);
        G4RunManager::GetRunManager()->ReinitializeGeometry();
    }
}

// ======================================================
// Messenger — define los comandos de macro
// ======================================================
void DetectorConstruction::DefineCommands()
{
    fMessenger = new G4GenericMessenger(this, "/detector/",
                                        "Parametros de geometria del detector");

    fMessenger->DeclareMethodWithUnit("grapheneThickness", "um",
        &DetectorConstruction::SetGrapheneThickness,
        "Espesor del film de grafeno [um]")
        .SetParameterName("thickness", false)
        .SetRange("thickness > 0");

    fMessenger->DeclareMethodWithUnit("kaptonThickness", "um",
        &DetectorConstruction::SetKaptonThickness,
        "Espesor del film de Kapton [um]")
        .SetParameterName("thickness", false)
        .SetRange("thickness > 0");

    fMessenger->DeclareMethod("boronFraction",
        &DetectorConstruction::SetBoronFraction,
        "Fracción másica de B-10 en el grafeno (0–1, e.g. 0.05 = 5 %)")
        .SetParameterName("fraction", false)
        .SetRange("fraction > 0 && fraction < 1");
}

G4VPhysicalVolume* DetectorConstruction::Construct() {
    auto nist = G4NistManager::Instance();

    G4cout << "\n[Geometria] Grafeno: " << fGrapheneThickness/um << " um"
           << "  |  Kapton: " << fKaptonThickness/um << " um\n" << G4endl;

    // --- Mundo ---
    auto worldMat = nist->FindOrBuildMaterial("G4_AIR");
    auto solidWorld = new G4Box("World", 10*cm, 10*cm, 10*cm);
    auto logicWorld = new G4LogicalVolume(solidWorld, worldMat, "World");
    auto physWorld  = new G4PVPlacement(0, G4ThreeVector(), logicWorld, "World", 0, false, 0);

    // --- Film Kapton (ANTES del grafeno — el neutrón lo atraviesa primero) ---
    G4double grapheneHalfZ = fGrapheneThickness / 2.0;
    G4double kaptonHalfZ   = fKaptonThickness   / 2.0;
    G4Material* kaptonMat  = nist->FindOrBuildMaterial("G4_KAPTON");

    G4double kaptonZ = -(grapheneHalfZ + kaptonHalfZ);
    auto solidkapton = new G4Box("kapton", 1.5*cm, 1.5*cm, kaptonHalfZ);
    auto logickapton = new G4LogicalVolume(solidkapton, kaptonMat, "kapton");
    new G4PVPlacement(0, G4ThreeVector(0,0,kaptonZ), logickapton, "kapton", logicWorld, false, 0);

    // --- Film de grafeno (fGrapheneMat creado en DefineMaterials) ---
    auto solidgraphene = new G4Box("graphene", 1*cm, 1*cm, grapheneHalfZ);
    fLogicGraphene = new G4LogicalVolume(solidgraphene, fGrapheneMat, "graphene");
    new G4PVPlacement(0, G4ThreeVector(0,0,0), fLogicGraphene, "graphene", logicWorld, false, 0);

    // --- Detector plano (después del grafeno) ---
    G4double detThickness = 0.5*cm;
    G4double detHalfZ     = detThickness / 2.0;
    G4double detZ         = grapheneHalfZ + detHalfZ;
    auto solidDet = new G4Box("Detector", 1.5*cm, 1.5*cm, detHalfZ);
    fLogicDet = new G4LogicalVolume(solidDet, nist->FindOrBuildMaterial("G4_AIR"), "Detector");
    new G4PVPlacement(0, G4ThreeVector(0,0,detZ), fLogicDet, "Detector", logicWorld, false, 0);

    // --- Límites de paso ---
    fLogicGraphene->SetUserLimits(new G4UserLimits(0.1*um));
    logickapton->SetUserLimits(new G4UserLimits(0.01*mm));
    fLogicDet->SetUserLimits(new G4UserLimits(0.01*mm));

    logicWorld->SetVisAttributes(G4VisAttributes::GetInvisible());

    // --- Cortes de producción ---
    // G4Region se busca primero; si ya existe (reinicialización) se reutiliza.
    // Sus root volumes se limpian automáticamente cuando los LVs son borrados.
    auto* regionStore = G4RegionStore::GetInstance();
    G4Region* region  = regionStore->GetRegion("DetectorRegion", false);
    if (!region) {
        region = new G4Region("DetectorRegion");
    }
    auto cuts = new G4ProductionCuts();
    cuts->SetProductionCut(0.01*mm, G4ProductionCuts::GetIndex("neutron"));
    cuts->SetProductionCut(0.01*mm, G4ProductionCuts::GetIndex("gamma"));
    cuts->SetProductionCut(0.01*mm, G4ProductionCuts::GetIndex("e-"));
    region->AddRootLogicalVolume(fLogicGraphene);
    region->AddRootLogicalVolume(logickapton);
    region->AddRootLogicalVolume(fLogicDet);
    region->SetProductionCuts(cuts);

    // --- Atributos visuales ---
    auto visGraphene = new G4VisAttributes(G4Colour(0.0, 0.0, 1.0, 0.7)); // Azul
    visGraphene->SetForceSolid(true);
    fLogicGraphene->SetVisAttributes(visGraphene);

    auto visKapton = new G4VisAttributes(G4Colour(1.0, 0.7, 0.3, 0.4)); // Naranja
    visKapton->SetForceSolid(true);
    logickapton->SetVisAttributes(visKapton);

    auto visDetector = new G4VisAttributes(G4Colour(0.0, 1.0, 0.0, 0.2)); // Verde
    visDetector->SetForceSolid(true);
    fLogicDet->SetVisAttributes(visDetector);

    fGeometryBuilt = true;
    return physWorld;
}

void DetectorConstruction::ConstructSDandField() {
    G4SDManager* sdman = G4SDManager::GetSDMpointer();

    // Los SDs se crean UNA sola vez (primera inicialización).
    // En reinicializaciones, fCaptureSD y fTransmittedSD ya están válidos
    // y solo se reasignan a los nuevos volúmenes lógicos.
    if (!fCaptureSD) {
        fCaptureSD = new CaptureSD("GrapheneSD", "GrapheneHitsCollection");
        sdman->AddNewDetector(fCaptureSD);
    }
    if (fLogicGraphene) {
        fLogicGraphene->SetSensitiveDetector(fCaptureSD);
    } else {
        G4Exception("DetectorConstruction::ConstructSDandField", "SD001",
                    JustWarning, "fLogicGraphene is null.");
    }

    if (!fTransmittedSD) {
        fTransmittedSD = new TransmittedSD("TransmittedSD");
        sdman->AddNewDetector(fTransmittedSD);
    }
    if (fLogicDet) {
        fLogicDet->SetSensitiveDetector(fTransmittedSD);
    } else {
        G4Exception("DetectorConstruction::ConstructSDandField", "SD002",
                    JustWarning, "fLogicDet is null.");
    }
}
