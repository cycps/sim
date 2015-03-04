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
Link::Link(SymbolSP name, size_t line, size_t column) : Element(name, line, column)
{
  params.push_back(make_shared<Symbol>("Latency", name->line, name->column));
  params.push_back(make_shared<Symbol>("Bandwidth", name->line, name->column));
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

//Cypress::CxxResidualFuncBuilder ---------------------------------------------

//Assumes eqtn is already in residual form e.g., 0 = f(x);
string CxxResidualFuncBuilder::run(EquationSP eqtn, size_t idx)
{
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
  string sname = s->value;
  boost::replace_all(sname, ".", "_");
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


//Controlled variable extraction ----------------------------------------------

void CVarExtractor::run(ComponentSP c, EquationSP e)
{
  component = c;  
  e->accept(*this);
}

void CVarExtractor::visit(CVarSP)
{
  inCVar = true;
}

void CVarExtractor::leave(CVarSP)
{
  inCVar = false;
}
  
void CVarExtractor::visit(DifferentiateSP)
{
  inDeriv = true;
}
  
void CVarExtractor::leave(DifferentiateSP)
{
  inDeriv = false;
}

void CVarExtractor::in(SymbolSP s)
{
  if(inCVar)
  {
    /*
    if(inDeriv) cderivs.insert(s->value);
    else cvars.insert(s->value);
    */
    if(inDeriv) cderivs.insert({component, s->value});
    else cvars.insert({component, s->value});
  }
}
