#ifndef DetectorConstruction_h
#define DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"
#include "G4SystemOfUnits.hh"
#include "G4Threading.hh"
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
    void SetOxygenFraction(G4double f);
    void SetHydrogenFraction(G4double f);
    void SetGrapheneDensity(G4double d);
    void SetOrder(const G4String& order);
    void SetGrapheneEnabled(G4bool enabled);

private:
    void DefineMaterials();
    void DefineCommands();
    void BuildGrapheneMaterial();

    G4double    fGrapheneThickness = 5*um;
    G4double    fKaptonThickness   = 9*um;
    G4double    fBoronFraction     = 0.05;
    G4double    fOxygenFraction    = 0.0;   // fraccion masica de O (grafeno oxidado)
    G4double    fHydrogenFraction  = 0.0;   // fraccion masica de H (-OH, agua intercalada)
    G4double    fGrapheneDensity   = 2.2*g/cm3;  // baja si hay oxidacion (mayor espaciado interlaminar)
    G4bool      fGrapheneEnabled   = true;
    G4bool      fKaptonFirst       = true;   // true = kapton antes del grafeno
    G4Material* fGrapheneMat       = nullptr;
    G4Element*  fElB_enriched      = nullptr;
    G4bool      fGeometryBuilt     = false;

    // G4ThreadLocal (no compartidos): en Geant4 MT, ConstructSDandField() se
    // llama UNA VEZ POR HILO WORKER y cada hilo necesita su PROPIA instancia
    // de sensitive detector (fHitsCollection, contadores, etc. son estado
    // mutable por-evento). Como DetectorConstruction es un unico objeto
    // compartido entre todos los hilos, si estos punteros fueran miembros
    // normales, el patron "if (!fCaptureSD) fCaptureSD = new CaptureSD(...)"
    // haria que SOLO el primer hilo worker cree su SD y los otros 11 hilos
    // reutilicen esa MISMA instancia compartida -> condicion de carrera sobre
    // fHitsCollection (multiples hilos procesando eventos distintos en
    // paralelo, leyendo/escribiendo el mismo puntero) -> SIGSEGV esporadico
    // en CaptureSD::ProcessHits una vez que ocurre una captura real.
    static G4ThreadLocal CaptureSD*     fCaptureSD;
    static G4ThreadLocal TransmittedSD* fTransmittedSD;

    G4LogicalVolume* fLogicGraphene = nullptr;
    G4LogicalVolume* fLogicDet      = nullptr;

    G4GenericMessenger* fMessenger = nullptr;
};

#endif
