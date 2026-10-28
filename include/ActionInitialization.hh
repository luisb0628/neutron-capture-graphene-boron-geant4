#ifndef ActionInitialization_h
#define ActionInitialization_h 1

#include "G4VUserActionInitialization.hh"

class GeneratorMessenger;

class ActionInitialization : public G4VUserActionInitialization {
  public:
    ActionInitialization();
    ~ActionInitialization() override;

    void BuildForMaster() const override;
    void Build() const override;

  private:
    // Construido aqui (una sola vez, antes de /run/initialize) para que el
    // comando /generator/phaseSpaceFile exista cuando el macro lo use -- ver
    // PhaseSpaceConfig.hh/GeneratorMessenger.hh.
    GeneratorMessenger* fGeneratorMessenger;
};

#endif
