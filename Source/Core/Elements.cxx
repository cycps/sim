#include "Cypress/Core/Elements.hxx"

using namespace cypress;
using std::shared_ptr;

//Free functions over elements ================================================
void cypress::setEqtnsToZero(shared_ptr<Element> e)
{
  for(auto eqtn : e->eqtns) setToZero(eqtn);
}

shared_ptr<Element> cypress::qualifyEqtns(shared_ptr<Element> e)
{
  return e;
}

// EqtnQualifier --------------------------------------------------------------
void EqtnQualifier::setQualifier(shared_ptr<Component> c)
{
  qual = c;
}

void EqtnQualifier::visit(shared_ptr<Symbol> s)
{
  s->value = qual->name->value + "." + s->value;
}

void EqtnQualifier::run(shared_ptr<Equation> eqtn)
{
  eqtn->accept(*this);
}
