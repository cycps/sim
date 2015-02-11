#include "Cypress/Core/Elements.hxx"
#include <vector>

using namespace cypress;
using std::vector;
using std::make_shared;

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

//Link ------------------------------------------------------------------------
Link::Link(SymbolSP name) : Element(name)
{
  params.push_back(make_shared<Symbol>("Latency", name->line));
  params.push_back(make_shared<Symbol>("Bandwidth", name->line));
}
