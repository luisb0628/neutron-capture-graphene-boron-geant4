#ifndef RUNACTION_HH
#define RUNACTION_HH

#include "G4UserRunAction.hh"
#include "globals.hh"
#include <fstream>

class RunAction : public G4UserRunAction
{
public:
    RunAction();
    ~RunAction() override;

    void BeginOfRunAction(const G4Run*) override;
    void EndOfRunAction(const G4Run*) override;

    // Contador de capturas neutrónicas (una captura = una alpha registrada)
    void IncrementCaptures() { ++fNeutronsCaptured; }
    G4int GetCaptureCount() const { return fNeutronsCaptured; }

    // Archivo ASCII de salida
    std::ofstream outputFile;

private:
    G4int fNeutronsCaptured = 0;
};

#endif
