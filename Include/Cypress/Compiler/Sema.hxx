#ifndef CYPRESS_COMPILE_SEMA
#define CYPRESS_COMPILE_SEMA

#include "Cypress/Core/Elements.hxx"
#include <unordered_map>
#include <unordered_set>
#include <sstream>

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

struct EqtnPrinter : public Visitor
{
  std::shared_ptr<Element> elem;
  void run(std::shared_ptr<Element> e, bool qualified=false);
  void run(std::shared_ptr<Equation> eqtn);
  bool qualified{false};
  std::stringstream ss;
  std::vector<std::string> strings;

  void in(std::shared_ptr<Equation>) override;
  void in(std::shared_ptr<Add>) override;
  void in(std::shared_ptr<Subtract>) override;
  void in(std::shared_ptr<Multiply>) override;
  void in(std::shared_ptr<Divide>) override;
  void in(std::shared_ptr<Symbol>) override;
  void in(std::shared_ptr<Pow>) override;
  void in(std::shared_ptr<Real>) override;
  void in(std::shared_ptr<Differentiate>) override;
  void visit(std::shared_ptr<SubExpression>) override;
  void leave(std::shared_ptr<SubExpression>) override;
  void visit(CVarSP) override;
  void leave(CVarSP) override;
};


}}

#endif
