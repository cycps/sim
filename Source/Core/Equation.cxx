#include "Cypress/Core/Equation.hxx"
#include <iostream>

using namespace cypress;
using std::shared_ptr;
using std::make_shared;
using std::string;
using std::cout;
using std::endl;

//Core data structures ========================================================

//Add -------------------------------------------------------------------------
void Add::accept(Visitor &v)
{
  v.visit(shared_from_this());
  lhs->accept(v);
  v.in(shared_from_this());
  rhs->accept(v);
  v.leave(shared_from_this());
}

std::shared_ptr<Expression> Add::clone()
{
  return std::make_shared<Add>(lhs->clone(), rhs->clone());
}

//Subtract --------------------------------------------------------------------
void Subtract::accept(Visitor &v)
{
  v.visit(shared_from_this());
  lhs->accept(v);
  v.in(shared_from_this());
  rhs->accept(v);
  v.leave(shared_from_this());
}

std::shared_ptr<Expression> Subtract::clone()
{
  return std::make_shared<Subtract>(lhs->clone(), rhs->clone());
}

//Multiply --------------------------------------------------------------------
void Multiply::accept(Visitor &v)
{
  v.visit(shared_from_this());
  lhs->accept(v);
  v.in(shared_from_this());
  rhs->accept(v);
  v.leave(shared_from_this());
}

std::shared_ptr<Expression> Multiply::clone()
{
  return std::make_shared<Multiply>(
      std::static_pointer_cast<Term>(lhs->clone()), 
      std::static_pointer_cast<Term>(rhs->clone())
      );
}

//Divide ----------------------------------------------------------------------
void Divide::accept(Visitor &v)
{
  v.visit(shared_from_this());
  lhs->accept(v);
  v.in(shared_from_this());
  rhs->accept(v);
  v.leave(shared_from_this());
}

std::shared_ptr<Expression> Divide::clone()
{
  return std::make_shared<Multiply>(
      std::static_pointer_cast<Term>(lhs->clone()), 
      std::static_pointer_cast<Term>(rhs->clone())
      );
}

//Pow -------------------------------------------------------------------------
void Pow::accept(Visitor &v)
{
  v.visit(shared_from_this());
  lhs->accept(v);
  v.in(shared_from_this());
  rhs->accept(v);
  v.leave(shared_from_this());
}

std::shared_ptr<Expression> Pow::clone()
{
  return std::make_shared<Pow>(
      std::static_pointer_cast<Atom>(lhs->clone()), 
      std::static_pointer_cast<Atom>(rhs->clone())
      );
}

//Symbol ----------------------------------------------------------------------
void Symbol::accept(Visitor &v)
{
  v.visit(shared_from_this());
  v.in(shared_from_this());
  v.leave(shared_from_this());
}
std::shared_ptr<Expression> Symbol::clone()
{
  return std::make_shared<Symbol>(value);
}

size_t SymbolHash::operator()(std::shared_ptr<Symbol> a)
{
  return hsh(a->value);
}

bool SymbolEq::operator()(std::shared_ptr<Symbol> a, std::shared_ptr<Symbol> b)
{
  return sh(a) == sh(b);
}

//Differentiate ---------------------------------------------------------------
void Differentiate::accept(Visitor &v)
{
  v.visit(shared_from_this());
  arg->accept(v);
  v.in(shared_from_this());
  v.leave(shared_from_this());
}

std::shared_ptr<Expression> Differentiate::clone()
{
  return std::make_shared<Differentiate>(
      std::static_pointer_cast<Symbol>(arg->clone())
      );
}

//Real ------------------------------------------------------------------------
void Real::accept(Visitor &v)
{
  v.visit(shared_from_this());
  v.in(shared_from_this());
  v.leave(shared_from_this());
}

std::shared_ptr<Expression> Real::clone()
{
  return std::make_shared<Real>(value);
}

//SubExpression ---------------------------------------------------------------
void SubExpression::accept(Visitor &v)
{
  v.visit(shared_from_this());
  value->accept(v);
  v.in(shared_from_this());
  v.leave(shared_from_this());
}

std::shared_ptr<Expression> SubExpression::clone()
{
  return std::make_shared<SubExpression>(
      std::static_pointer_cast<Expression>(value->clone())
      );
}

//Equation --------------------------------------------------------------------
void Equation::accept(Visitor &v) 
{
  v.visit(shared_from_this());
  lhs->accept(v);
  v.in(shared_from_this());
  rhs->accept(v);
  v.leave(shared_from_this());
}

std::shared_ptr<Equation> Equation::clone()
{
  auto cln = std::make_shared<Equation>();
  cln->lhs = lhs->clone();
  cln->rhs = rhs->clone();
  return cln;
}


//Free functions over equations ===============================================
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

void cypress::applyParameter(shared_ptr<Equation> eq, string symbol_name, 
    double value)
{
  EqtnParametizer eqp;
  eqp.symbol_name = symbol_name;
  eqp.value = value;

  eq->accept(eqp);

  std::cout << symbol_name << " -> " << value << std::endl;
}

// EqtnParametizer ------------------------------------------------------------

void EqtnParametizer::visit(shared_ptr<Add> ap)
{
  apply(ap);
}

void EqtnParametizer::visit(shared_ptr<Subtract> sp)
{
  apply(sp);
}

void EqtnParametizer::visit(shared_ptr<Multiply> mp)
{
  apply(mp);
}

void EqtnParametizer::visit(shared_ptr<Divide> dp)
{
  apply(dp);
}

void EqtnParametizer::visit(shared_ptr<Pow> pp)
{
  apply(pp);
}
