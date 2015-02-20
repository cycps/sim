#ifndef CYPRESS_COMPILE_SEMA
#define CYPRESS_COMPILE_SEMA

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


// Diagnostics ================================================================
struct Diagnostic
{
  enum class Level : int { Error=0, Warning=1, Info=2 };
  Level level{Level::Info};
  std::string message;
  size_t line{0};
  Diagnostic(Level level, std::string message, size_t line)
    : level{level}, message{message}, line{line} 
  {}
};

struct DiagnosticReport
{
  std::vector<Diagnostic> diagnostics;
  Diagnostic::Level level{Diagnostic::Level::Info};
  bool catastrophic();
};
 
std::ostream& operator<<(std::ostream &, const Diagnostic &);
std::ostream& operator<<(std::ostream &, const DiagnosticReport &);

struct CompilationError : public std::exception
{
  DiagnosticReport report;

  CompilationError(DiagnosticReport dr) : report(dr) {}

  const char* what() const noexcept override;
  
  private:
   mutable std::string what_;
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

}}

#endif
