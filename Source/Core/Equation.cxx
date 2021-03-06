#include "Cypress/Core/Equation.hxx"
#include <iostream>
#include <sstream>

using namespace cypress;
using std::shared_ptr;
using std::make_shared;
using std::string;
using std::cout;
using std::endl;
using std::static_pointer_cast;

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

ExpressionSP Add::clone()
{
  return make_shared<Add>(lhs->clone(), rhs->clone(), line, column);
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

ExpressionSP Subtract::clone()
{
  return make_shared<Subtract>(lhs->clone(), rhs->clone(), line, column);
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

ExpressionSP Multiply::clone()
{
  return make_shared<Multiply>(
      static_pointer_cast<Term>(lhs->clone()), 
      static_pointer_cast<Term>(rhs->clone()),
      line, column
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

ExpressionSP Divide::clone()
{
  return make_shared<Divide>(
      static_pointer_cast<Term>(lhs->clone()), 
      static_pointer_cast<Term>(rhs->clone()),
      line, column
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

ExpressionSP Pow::clone()
{
  return make_shared<Pow>(
      static_pointer_cast<Atom>(lhs->clone()), 
      static_pointer_cast<Atom>(rhs->clone()),
      line, column
      );
}

//Symbol ----------------------------------------------------------------------
void Symbol::accept(Visitor &v)
{
  v.visit(shared_from_this());
  v.in(shared_from_this());
  v.leave(shared_from_this());
}

ExpressionSP Symbol::clone()
{
  return make_shared<Symbol>(value, line, column);
}

size_t SymbolHash::operator()(SymbolSP a)
{
  static const std::hash<std::string> hsh{};
  return hsh(a->value);
}

bool SymbolEq::operator()(SymbolSP a, SymbolSP b)
{
  return sh(a) == sh(b);
}

//CVar ------------------------------------------------------------------------
void CVar::accept(Visitor &v)
{
  v.visit(shared_from_this());
  value->accept(v);
  v.in(shared_from_this());
  v.leave(shared_from_this());
}

ExpressionSP CVar::clone()
{
  return make_shared<CVar>(static_pointer_cast<Symbol>(value->clone()));
}

//CCVar ------------------------------------------------------------------------
void CCVar::accept(Visitor &v)
{
  v.visit(shared_from_this());
  value->accept(v);
  v.in(shared_from_this());
  v.leave(shared_from_this());
}

ExpressionSP CCVar::clone()
{
  return make_shared<CCVar>(static_pointer_cast<Symbol>(value->clone()));
}

//BoundVar --------------------------------------------------------------------
void BoundVar::accept(Visitor &v)
{
  v.visit(shared_from_this());
  value->accept(v);
  v.in(shared_from_this());
  v.leave(shared_from_this());
}

ExpressionSP BoundVar::clone()
{
  auto bv = make_shared<BoundVar>(static_pointer_cast<Symbol>(value->clone()));
  bv->bound = bound;
  return bv;
}

//IOVar -----------------------------------------------------------------------
void IOVar::accept(Visitor &v)
{
  v.visit(shared_from_this());
  value->accept(v);
  v.in(shared_from_this());
  v.leave(shared_from_this());
}

ExpressionSP IOVar::clone()
{
  auto iov = make_shared<IOVar>(static_pointer_cast<Symbol>(value->clone()));
  iov->iokind = iokind;
  return iov;
}

//Differentiate ---------------------------------------------------------------
void Differentiate::accept(Visitor &v)
{
  v.visit(shared_from_this());
  arg->accept(v);
  v.in(shared_from_this());
  v.leave(shared_from_this());
}

ExpressionSP Differentiate::clone()
{
  return make_shared<Differentiate>(static_pointer_cast<Symbol>(arg->clone()), 
      line, column);
}

//Real ------------------------------------------------------------------------
void Real::accept(Visitor &v)
{
  v.visit(shared_from_this());
  v.in(shared_from_this());
  v.leave(shared_from_this());
}

ExpressionSP Real::clone()
{
  return make_shared<Real>(value, line, column);
}

//SubExpression ---------------------------------------------------------------
void SubExpression::accept(Visitor &v)
{
  v.visit(shared_from_this());
  value->accept(v);
  v.in(shared_from_this());
  v.leave(shared_from_this());
}

ExpressionSP SubExpression::clone()
{
  return make_shared<SubExpression>(
      static_pointer_cast<Expression>(value->clone()));
}


//Equation --------------------------------------------------------------------
void Equation::accept(Visitor &v) 
{
  v.visit(shared_from_this());
  if(lhs == nullptr) 
    throw std::runtime_error("lhs lost"); 

  lhs->accept(v);

  v.in(shared_from_this());

  /*
  if(rhs == nullptr) 
    throw std::runtime_error("rhs lost"); 
    */

  if(rhs != nullptr)
  {
    rhs->accept(v);
    v.leave(shared_from_this());
  }
}

EquationSP Equation::clone()
{
  auto cln = make_shared<Equation>(line, column);
  cln->lhs = lhs->clone();
  if(rhs != nullptr)
    cln->rhs = rhs->clone();
  else
    cln->rhs = nullptr;
  return cln;
}

BoundSP Bound::clone()
{
  auto cln = make_shared<Bound>(line, column);
  cln->lhs = std::dynamic_pointer_cast<VarType>(lhs->clone());
  cln->rhs = std::dynamic_pointer_cast<Real>(rhs->clone());
  cln->kind = kind;
  return cln;
}


//Free functions over equations ===============================================
EquationSP cypress::setToZero(EquationSP eq)
{
  auto new_rhs = make_shared<Subtract>(
      make_shared<SubExpression>(eq->lhs), 
      make_shared<SubExpression>(eq->rhs),
      eq->line, eq->column
      );

  eq->lhs = make_shared<Real>(0, eq->line, eq->column);
  eq->rhs = new_rhs;
  return eq;
}

//EqtnParametizer -------------------------------------------------------------
void EqtnParametizer::visit(AddSP ap)
{
  apply(ap);
}

void EqtnParametizer::visit(SubtractSP sp)
{
  apply(sp);
}

void EqtnParametizer::visit(MultiplySP mp)
{
  apply(mp);
}

void EqtnParametizer::visit(DivideSP dp)
{
  apply(dp);
}

void EqtnParametizer::visit(PowSP pp)
{
  apply(pp);
}

void cypress::applyParameter(EquationSP eq, string symbol_name, double value)
{
  EqtnParametizer eqp;
  eqp.symbol_name = symbol_name;
  eqp.value = value;

  eq->accept(eqp);
}


void cypress::liftControlledVars(EquationSP eq, string symbol_name)
{
  VarLifter<CVar> cvl(symbol_name);
  eq->accept(cvl);
}

