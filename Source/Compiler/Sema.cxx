#include "Cypress/Compiler/Sema.hxx"
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include <set>

using namespace cypress;
using namespace cypress::compile;
using std::find_if;
using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;
using std::make_shared;
using std::runtime_error;
using std::stringstream;
using std::shared_ptr;
using std::set_difference;
using std::pair;
using std::transform;
using std::back_inserter;
using std::inserter;
using std::set;

void VarCollector::run(ElementSP e)
{
  elem = e;
  for(EquationSP eqtn : elem->eqtns) eqtn->accept(*this);
}

void VarCollector::visit(SymbolSP s)
{
   if(dblock) return; 
   if(find_if(elem->params.begin(), elem->params.end(), 
         [s](SymbolSP x){ return s->value == x->value; })
         != elem->params.end())
     return;

   vars[elem].insert(s); 
}

void VarCollector::leave(DifferentiateSP)
{
  dblock = false;
}

void VarCollector::visit(DifferentiateSP s)
{
  //TODO dupcheck?
  derivs[elem].insert(s->arg);
  dblock = true;
}

void VarCollector::showVars()
{
  cout << "vars:" << endl;
  for(auto p : vars)
    for(auto v : p.second)
      cout << p.first->name->value << "." << v->value << endl;
  cout << endl;
}

void VarCollector::showDerivs()
{
  cout << "derivs:" << endl;
  for(auto p : derivs)
    for(auto d : p.second)
      cout << p.first->name->value << "." << d->value << endl;
  cout << endl;
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

// Diagnostics ================================================================

bool DiagnosticReport::catastrophic()
{
  for(const Diagnostic d: diagnostics)
    if(d.level == Diagnostic::Level::Error)
      return true;

  return false;
}

const char* CompilationError::what() const noexcept
{
  stringstream ss;
  ss << report;
  what_ = ss.str();
  return what_.c_str();
}

ostream& cypress::compile::operator<<(ostream &o, const Diagnostic &d)
{
  string sev;
  switch(d.level)
  {
    case Diagnostic::Level::Error: sev = "Error"; break;
    case Diagnostic::Level::Warning: sev = "Warning"; break;
    case Diagnostic::Level::Info: sev = "Info"; break;
  }
  o << "[" << sev << "]:" << d.line+1 << " " << d.message << endl;
  
  return o;
}

ostream& cypress::compile::operator<<(ostream &o, const DiagnosticReport &dr)
{
  for(Diagnostic diag : dr.diagnostics)
    if(diag.level <= dr.level)
      o << diag;

  return o;
}

// Semantic Checks ============================================================

DiagnosticReport 
cypress::compile::check(ExperimentSP ex, vector<ElementSP> &elements)
{
  DiagnosticReport diags;

  for(ComponentSP c : ex->components)
  {
    check(c, elements, diags);
  }

  return diags;
}

DiagnosticReport&
cypress::compile::check(ComponentSP c, vector<ElementSP> &elements, 
    DiagnosticReport &dr)
{
  checkComponentType(c, elements, dr);
  checkComponentParams(c, dr);
  
  return dr;
}

DiagnosticReport&
cypress::compile::checkComponentType(ComponentSP c, 
    std::vector<ElementSP> &elements, DiagnosticReport &dr)
{
  if(c->kind->value == "Link") 
  {
    c->element = make_shared<Link>(c->name);
    return dr;
  }

  for(auto e : elements)
  {
    if(c->kind->value == e->name->value)
    {
      c->element = e;
      return dr;
    }
  }

  dr.diagnostics.push_back({
      Diagnostic::Level::Error,
      "Undefined Component Type: " + c->kind->value,
      c->kind->line
      });

  return dr;
}

DiagnosticReport&
cypress::compile::checkComponentParams(ComponentSP c, DiagnosticReport &dr)
{
  set<string> supplied;
  transform(c->params.begin(), c->params.end(), 
      inserter(supplied, supplied.begin()),
      [](pair<SymbolSP, RealSP> x){ return x.first->value; });

  set<string> required;
  transform(c->element->params.begin(), c->element->params.end(), 
      inserter(required, required.begin()),
      [](SymbolSP x){ return x->value; });

  vector<string> required_but_not_supplied;

  set_difference(
      required.begin(), required.end(),
      supplied.begin(), supplied.end(),
      back_inserter(required_but_not_supplied));

  if(!required_but_not_supplied.empty())
  {
    string missing_params{""};
    for(size_t i=0; i<required_but_not_supplied.size()-1; ++i)
      missing_params += required_but_not_supplied[i] + ", ";
    missing_params += required_but_not_supplied.back();

    string diag_string = 
      "The element " + c->element->name->value + " requires the parameters {" +
      missing_params + "}";

    dr.diagnostics.push_back({
        Diagnostic::Level::Error, diag_string, c->name->line});

  }

  vector<string> supplied_but_not_required;
  set_difference(
      supplied.begin(), supplied.end(),
      required.begin(), required.end(),
      back_inserter(supplied_but_not_required));

  if(!supplied_but_not_required.empty())
  {
    string extra_params{""};
    for(size_t i=0; i<supplied_but_not_required.size()-1; ++i)
      extra_params += supplied_but_not_required[i] + ", ";
    extra_params += supplied_but_not_required.back();

    string diag_string = 
      "The element " + c->element->name->value + " does not take the parameters {" +
      extra_params + "} they will be ignored";
    
    dr.diagnostics.push_back({
        Diagnostic::Level::Warning, diag_string, c->name->line});
  }

  return dr;
}

