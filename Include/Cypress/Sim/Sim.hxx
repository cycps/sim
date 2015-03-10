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

namespace cypress { namespace sim {

struct Sim; using SimSP = std::shared_ptr<Sim>;
struct SimEx; using SimExSP = std::shared_ptr<SimEx>;

struct Sim
{
  std::vector<std::shared_ptr<Object>> objects;
  std::shared_ptr<Experiment> exp;
  std::unordered_set<VarRefSP, VarRefSPHash, VarRefSPCmp> controlled_vars;
  std::unordered_set<VarRefSP, VarRefSPNameHash, VarRefSPNameCmp> vars;
  std::unordered_multimap<ComponentSP, EquationSP> psys;
  std::unordered_map<VarRefSP, double, VarRefSPNameHash, VarRefSPNameCmp> 
    initial_state,
    initial_trajectory;

  Sim(std::vector<std::shared_ptr<Object>>, std::shared_ptr<Experiment>);

  void buildPhysics();
  void buildSystemEquations();
  void buildSymbolSet();
  void buildInitials();
  void addCVarResiduals();
  void addObjectToSim(ComponentSP);
  void liftControlledSimVars(SubComponentRefSP);
  SimEx buildSimEx(size_t);
  std::vector<RVar> mapVariables(std::vector<ComputeNode> &);
  std::vector<REqtn> mapEquations(std::vector<ComputeNode> &);
  std::vector<ComputeNode> buildComputeTopology(size_t);
  std::string buildComputeNodeSource(const ComputeNode &);

};

struct EqtnVarCollector : Visitor
{
  std::unordered_set<VarRefSP, VarRefSPNameHash, VarRefSPNameCmp> vars;
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

}} //::cypress::sim

#endif
