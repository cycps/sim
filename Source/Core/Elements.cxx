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
