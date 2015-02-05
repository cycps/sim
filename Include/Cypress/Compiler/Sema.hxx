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
  std::unordered_set<std::shared_ptr<Symbol>, SymbolHash, SymbolEq> vars;
  //vector<shared_ptr<Symbol>> vars;
  void visit(shared_ptr<Symbol>) override;
};

}}

#endif
