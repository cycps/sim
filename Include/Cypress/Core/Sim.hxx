#ifndef CYPRESS_SIM
#define CYPRESS_SIM

#include "Cypress/Core/Elements.hxx"

namespace cypress
{

struct Sim
{
  std::vector<std::shared_ptr<Object>> objects;
  std::vector<std::shared_ptr<Controller>> controllers;
  std::vector<std::shared_ptr<Element>> elements;
  std::shared_ptr<Experiment> exp;

  std::vector<std::shared_ptr<Equation>> psys; //physical system

  Sim(std::vector<std::shared_ptr<Object>>,
      std::vector<std::shared_ptr<Controller>>,
      std::shared_ptr<Experiment>);

  void buildPhysics();
  void buildSystemEquations();
  void addObjectToSim(ComponentSP);
  void addControllerToSim(ComponentSP);

  void addControllerRefToSim(SubComponentRefSP);

};

}

#endif
