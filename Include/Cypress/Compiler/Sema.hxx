#ifndef CYPRESS_COMPILE_SEMA
#define CYPRESS_COMPILE_SEMA

#include "AST.hxx"
#include <unordered_set>

namespace cypress { namespace compile {

struct VarCollector : public Visitor
{
  std::unordered_set<std::shared_ptr<Symbol>, SymbolHash, SymbolEq> vars, derivs;
  bool dblock{false};

  void visit(std::shared_ptr<Symbol>) override;

  void visit(std::shared_ptr<Differentiate>) override;
  void leave(std::shared_ptr<Differentiate>) override;
};

struct EqtnPrinter : public Visitor
{
  void in(std::shared_ptr<Equation>) override;
  void in(std::shared_ptr<Add>) override;
  void in(std::shared_ptr<Subtract>) override;
  void in(std::shared_ptr<Multiply>) override;
  void in(std::shared_ptr<Divide>) override;
  void in(std::shared_ptr<Symbol>) override;
  void in(std::shared_ptr<Pow>) override;
  void in(std::shared_ptr<Real>) override;
  void in(std::shared_ptr<Differentiate>) override;
};

}}

#endif
