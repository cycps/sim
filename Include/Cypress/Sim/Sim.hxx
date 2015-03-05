#ifndef CYPRESS_SIM
#define CYPRESS_SIM

#include "Cypress/Core/Elements.hxx"
#include "Cypress/Sim/SimEx.hxx"
#include "Cypress/Sim/Resolve.hxx"
#include "Cypress/Sim/ComputeNode.hxx"
#include "Cypress/Core/Var.hxx"

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
  std::shared_ptr<Experiment> exp;

  std::unordered_set<VarRefSP, VarRefSPHash, VarRefSPCmp>
    controlled_vars;

  std::unordered_set<VarRefSP, VarRefSPNameHash, VarRefSPNameCmp>
    vars;

  std::unordered_map<VarRefSP, double, VarRefSPHash, VarRefSPCmp> 
    initial_state,
    initial_trajectory;

  std::unordered_multimap<ComponentSP, EquationSP> psys;

  Sim(std::vector<std::shared_ptr<Object>>,
      std::vector<std::shared_ptr<Controller>>,
      std::shared_ptr<Experiment>);

  void buildPhysics();
  void buildSystemEquations();

  void applyComponentParameters();
  void buildSymbolSet();
  void buildInitials();

  void addCVarResiduals();
  void addObjectToSim(ComponentSP);
  void addControllerToSim(ComponentSP);

  void addControllerRefToSim(SubComponentRefSP);

  SimEx buildSimEx();
  std::vector<RVar> mapVariables(std::vector<ComputeNode> &);
  std::vector<REqtn> mapEquations(std::vector<ComputeNode> &);
  std::vector<ComputeNode> buildComputeTopology(size_t);
  std::string buildComputeNodeSource(const ComputeNode &);

};

struct EqtnVarCollector : Visitor
{
  std::unordered_set<VarRefSP, VarRefSPHash, VarRefSPCmp> vars;
  ComponentSP component;
  void run(ComponentSP);

  bool in_derivative{false}, in_cvar{false};
  size_t derivative_order;

  private:
    void in(SymbolSP) override;
    void visit(DifferentiateSP) override;
    void leave(DifferentiateSP) override;
    void visit(CVarSP) override;
    void leave(CVarSP) override;
};

}

#endif
