#ifndef CYPRESS_COMPILE_SEMA
#define CYPRESS_COMPILE_SEMA

#include "AST.hxx"
#include <set>

namespace cypress { namespace compile {

struct VarCollector : public Visitor
{
  std::set<std::shared_ptr<Symbol>, SymbolCompare> vars;
  void visit(std::shared_ptr<Symbol>) override;
};

}}

#endif
