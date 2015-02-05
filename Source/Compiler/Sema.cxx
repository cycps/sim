#include "Cypress/Compiler/Sema.hxx"

using namespace cypress;
using namespace cypress::compile;

using std::shared_ptr;

void VarCollector::visit(shared_ptr<Symbol> s)
{
   vars.insert(s); 
   //vars.push_back(s);
}
