#include "Cypress/Core/Eqtn.hxx"

using namespace cypress;
using namespace cypress::compile;
using std::shared_ptr;
using std::make_shared;

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

shared_ptr<Element> cypress::qualifyEqtns(shared_ptr<Element> e)
{
  return e;
}
