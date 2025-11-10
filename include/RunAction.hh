#ifndef RUNACTION_HH
#define RUNACTION_HH

#include "G4UserRunAction.hh"
#include "globals.hh"
#include <fstream>   // <-- necesario para std::ofstream

class RunAction : public G4UserRunAction
{
public:
    RunAction();
    ~RunAction() override;

    void BeginOfRunAction(const G4Run*) override;
    void EndOfRunAction(const G4Run*) override;

    // === NUEVO ===
    // Archivo ASCII para guardar las partículas generadas (α, Li, γ)
    std::ofstream outputFile;
};

#endif
