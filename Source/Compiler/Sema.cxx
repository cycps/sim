#include "Cypress/Compiler/Sema.hxx"

using namespace cypress;
using namespace cypress::compile;

using std::shared_ptr;

void VarCollector::visit(shared_ptr<Symbol> s)
{
   if(!dblock) vars.insert(s); 
}

void VarCollector::leave(shared_ptr<Differentiate> s)
{
  dblock = false;
}

void VarCollector::visit(shared_ptr<Differentiate> s)
{
  derivs.insert(s->arg);
  dblock = true;
}
