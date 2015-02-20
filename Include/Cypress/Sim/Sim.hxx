#ifndef CYPRESS_SIM
#define CYPRESS_SIM

#include "Cypress/Core/Elements.hxx"
#include "Cypress/Sim/SimEx.hxx"
#include "Cypress/Sim/Resolve.hxx"
#include "Cypress/Sim/ComputeNode.hxx"

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

  std::vector<std::shared_ptr<Equation>> psys; //physical system

  Sim(std::vector<std::shared_ptr<Object>>,
      std::vector<std::shared_ptr<Controller>>,
      std::shared_ptr<Experiment>);

  void buildPhysics();
  void buildSystemEquations();
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

struct MetaVar
{
  std::string name;
  bool derivative, controlled;
  MetaVar()
    : derivative{false}, controlled{false}
  {}

  MetaVar(bool deriv, bool ctrl)
    : derivative{deriv}, controlled{ctrl}
  {}
};

struct MetaVarHash
{
  size_t operator()(const MetaVar &v)
  {
    return 
      std::hash<std::stsring>{}(v.name) + 
      std::hash<bool>{}(v.derivative) +
      std::hash<bool>{}(v.controlled);
  }
};

struct MetaVarCmp
{
  bool operator()(const MetaVar &a, const MetaVar &b)
  {
    return 
      a.name == b.name &&
      a.derivative == b.derivative &&
      a.controlled == b.controlled;
  }
};

struct EqtnVarCollector : public Visitor
{
  std::unordered_set<VarTraits, MetaVarhash, MetaVarCmp> vars;

  bool in_derivative{false}, in_cvar{false}, 
       explicit_derivs, include_cvar;

  EqtnVarCollector(bool explicit_derivs = true, bool include_cvar = true)
    : explicit_derivs{explicit_derivs}, include_cvar{include_cvar}
  {}

  void run(EquationSP);
  void in(SymbolSP) override;
  void visit(DifferentiateSP) override;
  void leave(DifferentiateSP) override;
  void visit(CVarSP) override;
  void leave(CVarSP) override;
};

}

#endif
