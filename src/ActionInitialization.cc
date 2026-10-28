#include "ActionInitialization.hh"
#include "PrimaryGeneratorAction.hh"
#include "RunAction.hh"
#include "EventAction.hh"
#include "SteppingAction.hh"   // ← agregar esto
#include "GeneratorMessenger.hh"

ActionInitialization::ActionInitialization()
: G4VUserActionInitialization(), fGeneratorMessenger(new GeneratorMessenger())
{}

ActionInitialization::~ActionInitialization()
{
    delete fGeneratorMessenger;
}

void ActionInitialization::BuildForMaster() const
{
  SetUserAction(new RunAction());
}

void ActionInitialization::Build() const
{
  SetUserAction(new PrimaryGeneratorAction());
  
  auto runAction = new RunAction();
  SetUserAction(runAction);
  SetUserAction(new EventAction(runAction));

  SetUserAction(new SteppingAction());
}
