#ifndef DetectorConstruction_h
#define DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"
#include "G4SystemOfUnits.hh"
#include "globals.hh"

class G4GenericMessenger;
class G4Material;
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

private:
    void DefineMaterials();
    void DefineCommands();

    G4double    fGrapheneThickness = 5*um;
    G4double    fKaptonThickness   = 9*um;
    G4Material* fGrapheneMat       = nullptr;
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
