#include "PrimaryGeneratorAction.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4PrimaryVertex.hh"
#include "G4PrimaryParticle.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "G4Event.hh"
#include "G4ios.hh"

// ROOT para leer el phase space
#include "TFile.h"
#include "TTree.h"
#include "TObjString.h"

PrimaryGeneratorAction::PrimaryGeneratorAction()
{
    LoadPhaseSpace("AmBePhaseSpace.root");

    if (fUseFallback) {
        // Sin archivo ROOT: pistola de neutrones térmicos como respaldo
        fFallbackGun = new G4ParticleGun(1);
        auto* def = G4ParticleTable::GetParticleTable()->FindParticle("neutron");
        fFallbackGun->SetParticleDefinition(def);
        fFallbackGun->SetParticleEnergy(0.025*eV);
        fFallbackGun->SetParticlePosition(G4ThreeVector(0., 0., -0.501*mm));
        fFallbackGun->SetParticleMomentumDirection(G4ThreeVector(0., 0., 1.));
        G4cout << "[PrimaryGenerator] Usando pistola de respaldo (neutrón térmico 0.025 eV).\n";
    } else {
        G4cout << "[PrimaryGenerator] Phase space cargado: "
               << fEntries.size() << " partículas desde AmBePhaseSpace.root\n";
    }
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
    delete fFallbackGun;
}

// ──────────────────────────────────────────────────────────────────────────────
// Lee todas las entradas del ntuple PhaseSpace en memoria.
// Cada entrada representa una partícula que llegó al detector de grafeno
// en la simulación Neutron_Thermalization.
// ──────────────────────────────────────────────────────────────────────────────
void PrimaryGeneratorAction::LoadPhaseSpace(const G4String& filename)
{
    TFile* file = TFile::Open(filename.c_str(), "READ");
    if (!file || file->IsZombie()) {
        G4cout << "[PrimaryGenerator] No se encontró " << filename
               << " — se usará pistola de respaldo.\n";
        fUseFallback = true;
        if (file) { file->Close(); delete file; }
        return;
    }

    TTree* tree = dynamic_cast<TTree*>(file->Get("PhaseSpace"));
    if (!tree) {
        G4cout << "[PrimaryGenerator] Ntuple 'PhaseSpace' no encontrado en "
               << filename << " — se usará pistola de respaldo.\n";
        fUseFallback = true;
        file->Close(); delete file;
        return;
    }

    char   particle[64];
    double kinE_eV, posX, posY, posZ, dirX, dirY, dirZ;

    tree->SetBranchAddress("Particle", particle);
    tree->SetBranchAddress("KinE_eV",  &kinE_eV);
    tree->SetBranchAddress("PosX_cm",  &posX);
    tree->SetBranchAddress("PosY_cm",  &posY);
    tree->SetBranchAddress("PosZ_cm",  &posZ);
    tree->SetBranchAddress("DirX",     &dirX);
    tree->SetBranchAddress("DirY",     &dirY);
    tree->SetBranchAddress("DirZ",     &dirZ);

    Long64_t nEntries = tree->GetEntries();
    fEntries.reserve(nEntries);

    for (Long64_t i = 0; i < nEntries; ++i) {
        tree->GetEntry(i);
        // Descartar partículas que van hacia atrás (DirZ <= 0)
        if (dirZ <= 0.) continue;
        fEntries.push_back({std::string(particle), kinE_eV, posX, posY, dirX, dirY, dirZ});
    }

    file->Close();
    delete file;

    if (fEntries.empty()) {
        G4cout << "[PrimaryGenerator] Phase space vacío — se usará pistola de respaldo.\n";
        fUseFallback = true;
    }
}

// ──────────────────────────────────────────────────────────────────────────────
// Un evento = una partícula del phase space.
// La posición Z se fija antes del Kapton; X e Y vienen del phase space.
// Al llegar al final del archivo, cicla desde el inicio.
// ──────────────────────────────────────────────────────────────────────────────
void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event)
{
    if (fUseFallback) {
        fFallbackGun->GeneratePrimaryVertex(event);
        return;
    }

    const PhaseSpaceEntry& e = fEntries[fCurrentEntry];
    fCurrentEntry = (fCurrentEntry + 1) % (G4int)fEntries.size();

    auto* table = G4ParticleTable::GetParticleTable();
    auto* def   = table->FindParticle(e.particle);
    if (!def) {
        G4cout << "[PrimaryGenerator] Partícula desconocida: " << e.particle
               << " (evento " << event->GetEventID() << ") — omitiendo.\n";
        return;
    }

    // Z fija 1 µm antes de la cara frontal del film (stack empieza en -0.5 mm)
    auto* vertex = new G4PrimaryVertex(
        e.posX_cm * cm, e.posY_cm * cm, -0.501*mm, 0.0);

    auto* primary = new G4PrimaryParticle(def);
    primary->SetKineticEnergy(e.kinE_eV * eV);
    primary->SetMomentumDirection(G4ThreeVector(e.dirX, e.dirY, e.dirZ));

    vertex->SetPrimary(primary);
    event->AddPrimaryVertex(vertex);
}
