#ifndef RUNACTION_HH
#define RUNACTION_HH

#include "G4UserRunAction.hh"
#include "globals.hh"

class G4Run;
class G4Timer; // Declaración adelantada para no incluir todo <G4Timer.hh> aquí

class RunAction : public G4UserRunAction
{
  public:
    RunAction();
    virtual ~RunAction() override;

    virtual void BeginOfRunAction(const G4Run*) override;
    virtual void EndOfRunAction(const G4Run*) override;

  private:
    G4Timer* fTimer; // 🔹 Temporizador para medir la duración del run
};

#endif
