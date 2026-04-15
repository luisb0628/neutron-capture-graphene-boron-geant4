#ifndef DetectorConstruction_h
#define DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"
#include "G4SystemOfUnits.hh"
#include "globals.hh"

class G4GenericMessenger;
class G4Material;
class G4Element;
class CaptureSD;
class TransmittedSD;

class DetectorConstruction : public G4VUserDetectorConstruction
{
public:
    DetectorConstruction();
    ~DetectorConstruction() override;

    G4VPhysicalVolume* Construct() override;
    void ConstructSDandField() override;

    void SetGrapheneThickness(G4double t);
    void SetKaptonThickness(G4double t);
    void SetBoronFraction(G4double f);

private:
    void DefineMaterials();
    void DefineCommands();
    void BuildGrapheneMaterial();

    G4double    fGrapheneThickness = 5*um;
    G4double    fKaptonThickness   = 9*um;
    G4double    fBoronFraction     = 0.05;   // fracción másica de B-10 (0–1)
    G4Material* fGrapheneMat       = nullptr;
    G4Element*  fElB_enriched      = nullptr;
    G4bool      fGeometryBuilt     = false;

    // SDs: creados una vez, reutilizados en cada reinit.
    CaptureSD*     fCaptureSD     = nullptr;
    TransmittedSD* fTransmittedSD = nullptr;

    // LVs: guardados en Construct() para evitar búsqueda por nombre en ConstructSDandField().
    G4LogicalVolume* fLogicGraphene = nullptr;
    G4LogicalVolume* fLogicDet      = nullptr;

    G4GenericMessenger* fMessenger = nullptr;
};

#endif
