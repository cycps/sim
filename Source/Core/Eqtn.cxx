#include "Cypress/Core/Eqtn.hxx"

using namespace cypress;
using namespace cypress::compile;
using std::shared_ptr;
using std::make_shared;
using std::string;

shared_ptr<Equation> cypress::setToZero(shared_ptr<Equation> eq)
{
  auto new_rhs = make_shared<Subtract>(
      make_shared<SubExpression>(eq->lhs), 
      make_shared<SubExpression>(eq->rhs)
      );

  eq->lhs = make_shared<Real>(0);
  eq->rhs = new_rhs;
  return eq;
}

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

void cypress::applyParameter(shared_ptr<Equation> eq, string symbol_name, double value)
{
  EqtnParametizer eqp;
  eqp.symbol_name = symbol_name;
  eqp.value = value;

  eq->accept(eqp);

  std::cout << symbol_name << " -> " << value << std::endl;
}

// EqtnParametizer ------------------------------------------------------------

void EqtnParametizer::visit(shared_ptr<compile::Add> ap)
{
  apply(ap);
}

void EqtnParametizer::visit(shared_ptr<compile::Subtract> sp)
{
  apply(sp);
}

void EqtnParametizer::visit(shared_ptr<compile::Multiply> mp)
{
  apply(mp);
}

void EqtnParametizer::visit(shared_ptr<compile::Divide> dp)
{
  apply(dp);
}

void EqtnParametizer::visit(shared_ptr<compile::Pow> pp)
{
  apply(pp);
}
