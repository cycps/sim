#ifndef CYPRESS_COMPILE_SEMA
#define CYPRESS_COMPILE_SEMA

#include "AST.hxx"
#include <unordered_set>
#include <vector>

using std::vector;
using std::shared_ptr;

namespace cypress { namespace compile {

struct VarCollector : public Visitor
{
  std::unordered_set<std::shared_ptr<Symbol>, SymbolHash, SymbolEq> vars, derivs;
  bool dblock{false};

  void visit(shared_ptr<Symbol>) override;

  void visit(shared_ptr<Differentiate>) override;
  void leave(shared_ptr<Differentiate>) override;
};

}}

#endif
