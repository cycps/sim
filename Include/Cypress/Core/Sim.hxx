#ifndef CYPRESS_SIM
#define CYPRESS_SIM

#include "Cypress/Compiler/AST.hxx"

namespace cypress
{

struct Sim
{
  std::vector<std::shared_ptr<compile::Object>> objects;
  std::vector<std::shared_ptr<compile::Controller>> controllers;
  std::vector<std::shared_ptr<compile::Element>> elements;
  std::shared_ptr<compile::Experiment> exp;

  std::vector<std::shared_ptr<compile::Equation>> psys; //physical system

  Sim(std::vector<std::shared_ptr<compile::Object>>,
      std::vector<std::shared_ptr<compile::Controller>>,
      std::shared_ptr<compile::Experiment>);

  void buildPhysics();
  void typeAssignComponents();
  void buildSystemEquations();

  std::shared_ptr<compile::Element> 
  findDecl(std::shared_ptr<compile::Component>);
};

}

#endif
