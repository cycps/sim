#ifndef CYPRESS_COMPILE_SEMA
#define CYPRESS_COMPILE_SEMA

#include "Cypress/Compiler/Diagnostics.hxx"
#include "Cypress/Core/Elements.hxx"
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <iostream>
#include <stdexcept>

namespace cypress { namespace compile {

struct VarCollector : public Visitor
{
  std::shared_ptr<Element> elem;
  std::unordered_map<
    std::shared_ptr<Element>,
    std::unordered_set<std::shared_ptr<Symbol>, SymbolHash, SymbolEq> 
  >
    vars, derivs;
  bool dblock{false};

  void run(std::shared_ptr<Element> e);

  void visit(std::shared_ptr<Symbol>) override;
  void visit(std::shared_ptr<Differentiate>) override;
  void leave(std::shared_ptr<Differentiate>) override;

  void showVars();
  void showDerivs();
};

// Semantic Checks ============================================================
DiagnosticReport check(ExperimentSP, std::vector<ElementSP>&);

DiagnosticReport& 
check(ComponentSP, std::vector<ElementSP>&, DiagnosticReport&);

DiagnosticReport&
checkComponentType(ComponentSP, std::vector<ElementSP>&, DiagnosticReport&);

DiagnosticReport&
checkComponentParams(ComponentSP, DiagnosticReport&);

DiagnosticReport&
checkConnection(ConnectionSP, std::vector<ComponentSP>&, DiagnosticReport&);

DiagnosticReport&
checkComponentRef(ComponentRefSP, std::vector<ComponentSP>&, DiagnosticReport&);

struct Sema
{
  SimulationSP sim;
  DiagnosticReportSP dr;
  std::vector<ObjectSP> objects;

  Sema(ExperimentSP, DiagnosticReportSP, std::vector<ObjectSP> objects);

  void check(); 

  void check(ObjectSP);
  //void inputCheck(ObjectSP);

  void check(ComponentSP);
  void typeCheck(ComponentSP);
  void paramsCheck(ComponentSP);

  void check(ConnectionSP);
  void check(ComponentRefSP);

};

}}

#endif
