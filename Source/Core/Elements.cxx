#include "Cypress/Core/Elements.hxx"
#include <boost/algorithm/string/replace.hpp>
#include <vector>
#include <stdexcept>
#include <algorithm>

using namespace cypress;
using std::vector;
using std::make_shared;
using std::string;
using std::runtime_error;
using std::find_if;
using std::pair;

ComponentSP Experiment::operator[](string s)
{
  auto it = find_if(components.begin(), components.end(),
      [s](ComponentSP c){ return c->name->value == s; });

  if(it != components.end()) return *it;

  throw runtime_error{"Experiment component not found: " + s};
}

Decls & Decls::operator += (const Decls &b)
{
  Decls &a = *this;

  a.objects.insert(
      a.objects.end(), 
      b.objects.begin(), 
      b.objects.end());

  a.controllers.insert(
      a.controllers.end(),
      b.controllers.begin(),
      b.controllers.end());

  a.experiments.insert(
      a.experiments.end(),
      b.experiments.begin(),
      b.experiments.end());

  return a;
}

// Component ------------------------------------------------------------------

RealSP Component::parameterValue(string s)
{
  auto it = 
    find_if(params.begin(), params.end(),
        [s](const pair<SymbolSP, RealSP> &p){ return p.first->value == s; });

  if(it != params.end())
    return it->second;

  throw runtime_error{"Component: Undefined parameter value requested: " + s};
}

RealSP Component::initialValue(string s, VarRef::Kind k)
{
  auto it =
    find_if(initials.begin(), initials.end(),
        [s,k](const pair<VarRefSP, RealSP> &p)
        { 
          return 
          p.first->name == s && 
          p.first->kind() == k;
        });

  if(it != initials.end())
    return it->second;

  string symb = s;
  if(k == VarRef::Kind::Derivative) symb += "'";

  throw runtime_error{"Component: Undefined initial value requested: " + symb};
}

void Component::applyParameters()
{
  for(auto p: params)
  {
    for(EquationSP eq: element->eqtns)
    {
      applyParameter(eq, p.first->value, p.second->value);
    }
  }
}


//Free functions over elements ================================================
void cypress::setEqtnsToZero(ElementSP e)
{
  for(auto eqtn : e->eqtns) setToZero(eqtn);
}

ElementSP cypress::qualifyEqtns(ElementSP e)
{
  return e;
}


//TODO: This should be a semantic action?
VarRefSP cypress::getDestination(ConnectableSP c)
{
  if(c->neighbor != nullptr) return getDestination(c->neighbor);

  if(c->kind() == Connectable::Kind::Component)
    throw runtime_error{"Control ends at component: `" +
      std::static_pointer_cast<ComponentRef>(c)->name->value + "`"};

  if(c->kind() == Connectable::Kind::AtoD)
    throw runtime_error{"Control ends at AtoD converter"};

  auto x = std::static_pointer_cast<SubComponentRef>(c);
  return make_shared<VarRef>(x->component, x->subname->value);
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
Link::Link(SymbolSP name, size_t line, size_t column) 
  : Element(name, line, column)
{
  params.push_back(make_shared<Symbol>("Latency", name->line, name->column));
  params.push_back(make_shared<Symbol>("Bandwidth", name->line, name->column));
}

//Actuator --------------------------------------------------------------------
Actuator::Actuator(SymbolSP name, size_t line, size_t column)
  : Element(name, line, column)
{
  size_t l = name->line, c = name->column;
  params.push_back(make_shared<Symbol>("Min", l, c));
  params.push_back(make_shared<Symbol>("Max", l, c));
  params.push_back(make_shared<Symbol>("DMin", l, c));
  params.push_back(make_shared<Symbol>("DMax", l, c));

  auto eq = make_shared<Equation>(-1, -1);
  eq->lhs = make_shared<Symbol>("u", -1, -1);
  //eq->rhs = make_shared<Symbol>("u", -1, -1);
  
  eqtns.push_back(eq);
}

// Eqtn Printer ---------------------------------------------------------------
void EqtnPrinter::run(ElementSP e, bool qualified)
{
  elem = e;
  this->qualified = qualified;
  for(EquationSP eqtn : elem->eqtns) 
  {
    eqtn->accept(*this);
    strings.push_back(ss.str());
    ss.str("");
  }
}

void EqtnPrinter::run(EquationSP eqtn)
{
  this->qualified = false;
  eqtn->accept(*this);
  strings.push_back(ss.str());
  ss.str("");
}

void EqtnPrinter::in(EquationSP)
{
  ss << " = ";
}
void EqtnPrinter::in(AddSP)
{
  ss << " + ";
}

void EqtnPrinter::in(SubtractSP)
{
  ss << " - ";
}

void EqtnPrinter::in(MultiplySP)
{
  ss << "*";
}

void EqtnPrinter::in(DivideSP)
{
  ss << "/";
}

void EqtnPrinter::in(SymbolSP sp)
{
  if(qualified)
    ss << elem->name->value << ".";
  ss << sp->value;
}

void EqtnPrinter::in(PowSP)
{
  ss << "^";
}

void EqtnPrinter::in(RealSP rp)
{
  ss << rp->value;
}

void EqtnPrinter::in(DifferentiateSP)
{
  ss << "'";
}

void EqtnPrinter::visit(SubExpressionSP)
{
  ss << " (";
}

void EqtnPrinter::leave(SubExpressionSP)
{
  ss << ") ";
}

void EqtnPrinter::visit(CVarSP)
{
  ss << "[";
}

void EqtnPrinter::leave(CVarSP)
{
  ss << "]";
}

void EqtnPrinter::visit(CCVarSP)
{
  ss << "{";
}

void EqtnPrinter::leave(CCVarSP)
{
  ss << "}";
}

#include <iostream>
void EqtnPrinter::visit(BoundVarSP x)
{
  string t;
  switch(x->bound->kind)
  {
    case Bound::Kind::LT: t = "<"; break;
    case Bound::Kind::AbsLT: t = "|<"; break;
    case Bound::Kind::GT: t = ">"; break;
    case Bound::Kind::AbsGT: t = "|>"; break;
      
  }
  ss << "[" << t << x->bound->rhs->value << "|";
}

void EqtnPrinter::leave(BoundVarSP)
{
  ss << "]";
}

void EqtnPrinter::visit(IOVarSP x)
{
  string t = x->iokind == IOVar::IOKind::Input ? "i" : "o";
  ss << "[" << t << "|";
}

void EqtnPrinter::leave(IOVarSP)
{
  ss << "]";
}

//Cypress::CxxResidualFuncBuilder ---------------------------------------------

//Assumes eqtn is already in residual form e.g., 0 = f(x);
string CxxResidualFuncBuilder::run(ComponentSP cp, EquationSP eqtn, size_t idx)
{
  this->cp = cp;
  ss.str("");
  //ss << "[this](){ return ";
  ss << "r[" << idx << "] = ";
  eqtn->rhs->accept(*this);
  ss << ";";
  return ss.str();
}

void CxxResidualFuncBuilder::in(AddSP) 
{ 
  ss << " + ";
}

void CxxResidualFuncBuilder::in(SubtractSP) 
{ 
  ss << " - ";
}

void CxxResidualFuncBuilder::in(MultiplySP) 
{ 
  ss << "*";
}

void CxxResidualFuncBuilder::in(DivideSP) 
{ 
  ss << "/";
}

void CxxResidualFuncBuilder::in(SymbolSP s) 
{ 
  string sname{""};
  if(qnames && cp != nullptr) sname += cp->name->value + "_";
  sname += s->value;
  ss << sname << "()";
}

void CxxResidualFuncBuilder::visit(PowSP) 
{ 
  ss << "pow(";
}

void CxxResidualFuncBuilder::in(PowSP) 
{ 
  ss << ",";
}

void CxxResidualFuncBuilder::leave(PowSP) 
{ 
  ss << ")";
}

void CxxResidualFuncBuilder::in(RealSP r) 
{ 
  ss << r->value;
}

void CxxResidualFuncBuilder::visit(DifferentiateSP) 
{ 
  ss << "d_";
}

void CxxResidualFuncBuilder::visit(SubExpressionSP) 
{ 
  ss << "(";
}

void CxxResidualFuncBuilder::leave(SubExpressionSP) 
{ 
  ss << ")";
}

void CxxResidualFuncBuilder::visit(CCVarSP)
{
  ss << "cx_";
}

void CxxResidualFuncBuilder::visit(IOVarSP v)
{
  if(v->iokind == IOVar::IOKind::Input) ss << "in_";
}

void CxxResidualFuncBuilder::visit(BoundVarSP v)
{
  switch(v->bound->kind)
  {
    case Bound::Kind::LT:
      ss << "cypress::bound_u(";
      break;

    case Bound::Kind::AbsLT:
      ss << "cypress::bound_au(";
      break;

    case Bound::Kind::GT:
      ss << "std::bound_l(";
      break;

    case Bound::Kind::AbsGT:
      ss << "std::bound_al(";
      break;
  }
}

void CxxResidualFuncBuilder::leave(BoundVarSP v)
{
  switch(v->bound->kind)
  {
    case Bound::Kind::LT:
      ss << "," << v->bound->rhs->value << ")";
      break;

    case Bound::Kind::AbsLT:
      ss << "," << v->bound->rhs->value << ")";
      break;

    case Bound::Kind::GT:
      ss << "," << v->bound->rhs->value << ")";
      break;

    case Bound::Kind::AbsGT:
      ss << "," << v->bound->rhs->value << ")";
      break;
  }

}


//Controlled variable extraction ----------------------------------------------

void VarExtractor::run(ComponentSP c, EquationSP e)
{
  component = c;  
  e->accept(*this);
}
  
void VarExtractor::run(ComponentSP c)
{
  component = c;
  for(EquationSP eqtn : c->element->eqtns) eqtn->accept(*this);
}

void VarExtractor::visit(CVarSP)
{
  ctx.cvar = true;
}

void VarExtractor::leave(CVarSP)
{
  ctx.cvar = false;
}
  
void VarExtractor::visit(DifferentiateSP)
{
  ctx.deriv = true;
}
  
void VarExtractor::leave(DifferentiateSP)
{
  ctx.deriv = false;
}

void VarExtractor::visit(IOVarSP v)
{
  switch(v->iokind)
  {
    case IOVar::IOKind::Input: ctx.input = true; break;
    case IOVar::IOKind::Output: ctx.output = true; break;
  }
}

void VarExtractor::leave(IOVarSP)
{
  ctx.input = ctx.output = false; 
}

void VarExtractor::in(SymbolSP s)
{
  if(filter(s, ctx))
  {
    if(ctx.deriv) vars.insert(make_shared<DVarRef>(component, s->value, 1));
    else vars.insert(make_shared<VarRef>(component, s->value));
  }
}
