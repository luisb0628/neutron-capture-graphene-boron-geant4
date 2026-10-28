#ifndef RUNACTION_HH
#define RUNACTION_HH

#include "G4UserRunAction.hh"
#include "G4Timer.hh"
#include "G4Threading.hh"
#include "globals.hh"

class RunAction : public G4UserRunAction
{
public:
    RunAction();
    ~RunAction() override;

    void BeginOfRunAction(const G4Run*) override;
    void EndOfRunAction(const G4Run*) override;

    // Contador de capturas neutrónicas (una captura = una alpha registrada).
    // Compartido entre hilos: en modo MT cada evento se procesa en un hilo
    // worker distinto, así que el conteo no puede vivir en un miembro por-instancia.
    static void IncrementCaptures();

private:
    G4Timer fTimer;

    static G4Mutex sCaptureMutex;
    static G4int   sTotalCaptures;
};

#endif
