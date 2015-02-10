#include "Cypress/Compiler/Sema.hxx"
#include <iostream>

using namespace cypress;
using namespace cypress::compile;
using std::find_if;
using std::cout;
using std::endl;

using std::shared_ptr;

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

