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

};

struct CxxResidualFuncBuilder : public Visitor
{
  std::stringstream ss;

  std::string run(EquationSP, size_t idx);

  void in(AddSP) override;
  void in(SubtractSP) override;
  void in(MultiplySP) override;
  void in(DivideSP) override;
  void in(SymbolSP) override;
  void visit(PowSP) override;
  void in(PowSP) override;
  void leave(PowSP) override;
  void in(RealSP) override;
  void visit(DifferentiateSP) override;
  void visit(SubExpressionSP) override;
  void leave(SubExpressionSP) override;
};

struct EqtnVarCollector : public Visitor
{
  std::unordered_set<std::string> vars;

  bool in_derivative;
  bool include_derivatives;

  EqtnVarCollector(bool inc_derivs = true)
    : include_derivatives{inc_derivs}
  {}

  void run(EquationSP);
  void in(SymbolSP) override;
  void visit(DifferentiateSP) override;
  void leave(DifferentiateSP) override;
};

}

#endif
