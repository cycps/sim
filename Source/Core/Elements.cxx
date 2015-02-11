#include "Cypress/Core/Elements.hxx"
#include <vector>

using namespace cypress;
using std::vector;

//Free functions over elements ================================================
void cypress::setEqtnsToZero(ElementSP e)
{
  for(auto eqtn : e->eqtns) setToZero(eqtn);
}

ElementSP cypress::qualifyEqtns(ElementSP e)
{
  return e;
}

vector<SubComponentRefSP>
cypress::findControlledSubComponents(ExperimentSP)
{
  vector<SubComponentRefSP> result;

  //TODO: When you get to links
  
  return result;
}

// EqtnQualifier --------------------------------------------------------------
void EqtnQualifier::setQualifier(ComponentSP c)
{
  qual = c;
}

void EqtnQualifier::visit(SymbolSP s)
{
  s->value = qual->name->value + "." + s->value;
}

void EqtnQualifier::run(EquationSP eqtn)
{
  eqtn->accept(*this);
}
