#ifndef CYPRESS_SIM
#define CYPRESS_SIM

#include "Cypress/Core/Elements.hxx"
#include "Cypress/Sim/SimEx.hxx"
#include "Cypress/Sim/Resolve.hxx"
#include "Cypress/Sim/ComputeNode.hxx"
#include "Cypress/Sim/Var.hxx"

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <sstream>

namespace cypress
{

struct Sim
{
  std::vector<std::shared_ptr<Object>> objects;
  std::vector<std::shared_ptr<Controller>> controllers;
  std::vector<std::shared_ptr<Element>> elements;
  std::shared_ptr<Experiment> exp;

  std::unordered_map<std::string, std::vector<MetaVar>> vref_map;

  std::vector<std::shared_ptr<Equation>> psys; //physical system

  Sim(std::vector<std::shared_ptr<Object>>,
      std::vector<std::shared_ptr<Controller>>,
      std::shared_ptr<Experiment>);

  void buildPhysics();
  void buildSystemEquations();
  void addCVarResiduals();
  void addObjectToSim(ComponentSP);
  void addControllerToSim(ComponentSP);

  void addControllerRefToSim(SubComponentRefSP);
  std::string buildResidualClosure();

  SimEx buildSimEx();
  std::vector<RVar> mapVariables(size_t);
  std::vector<REqtn> mapEquations(size_t);
  std::vector<ComputeNode> buildComputeTopology(size_t);
  std::string buildComputeNodeSource(const ComputeNode &);

};

struct EqtnVarCollector : public Visitor
{
  std::unordered_set<MetaVar, MetaVarHash, MetaVarCmp> vars;

  bool in_derivative{false}, in_cvar{false};

  void run(EquationSP);
  void in(SymbolSP) override;
  void visit(DifferentiateSP) override;
  void leave(DifferentiateSP) override;
  void visit(CVarSP) override;
  void leave(CVarSP) override;
};

}

#endif
