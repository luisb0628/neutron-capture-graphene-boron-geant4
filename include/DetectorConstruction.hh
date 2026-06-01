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
    void SetOrder(const G4String& order);
    void SetGrapheneEnabled(G4bool enabled);

private:
    void DefineMaterials();
    void DefineCommands();
    void BuildGrapheneMaterial();

    G4double    fGrapheneThickness = 5*um;
    G4double    fKaptonThickness   = 9*um;
    G4double    fBoronFraction     = 0.05;
    G4bool      fGrapheneEnabled   = true;
    G4bool      fKaptonFirst       = true;   // true = kapton antes del grafeno
    G4Material* fGrapheneMat       = nullptr;
    G4Element*  fElB_enriched      = nullptr;
    G4bool      fGeometryBuilt     = false;

    CaptureSD*     fCaptureSD     = nullptr;
    TransmittedSD* fTransmittedSD = nullptr;

    G4LogicalVolume* fLogicGraphene = nullptr;
    G4LogicalVolume* fLogicDet      = nullptr;

    G4GenericMessenger* fMessenger = nullptr;
};

#endif
