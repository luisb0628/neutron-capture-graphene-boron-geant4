#ifndef DetectorConstruction_h
#define DetectorConstruction_h 1

#include "G4VPhysicalVolume.hh"
#include "G4VUserDetectorConstruction.hh"

class DetectorConstruction : public G4VUserDetectorConstruction {
  public:
    DetectorConstruction();
    ~DetectorConstruction() override;
    G4VPhysicalVolume* Construct() override;
    void ConstructSDandField() override;
  private:
  // Variables para controlar la geometría (la que quieres modificar)
    G4double fgrapheneThickness; // Grosor del bloque (debe ser el doble del half-length)
    G4double fDetectorGap;       // Distancia entre parafina y detector
};

#endif
