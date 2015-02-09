#include "Cypress/Compiler/Sema.hxx"

using namespace cypress;
using namespace cypress::compile;
using std::find_if;
using std::cout;
using std::endl;

using std::shared_ptr;

void VarCollector::run(shared_ptr<Element> e)
{
  elem = e;
  for(shared_ptr<Equation> eqtn : elem->eqtns) eqtn->accept(*this);
}

void VarCollector::visit(shared_ptr<Symbol> s)
{
   if(dblock) return; 
   if(find_if(elem->params.begin(), elem->params.end(), 
         [s](shared_ptr<Symbol> x){ return s->value == x->value; })
         != elem->params.end())
     return;

   vars[elem].insert(s); 
}

void VarCollector::leave(shared_ptr<Differentiate>)
{
  dblock = false;
}

void VarCollector::visit(shared_ptr<Differentiate> s)
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
void EqtnPrinter::run(shared_ptr<Element> e, bool qualified)
{
  elem = e;
  this->qualified = qualified;
  for(shared_ptr<Equation> eqtn : elem->eqtns) 
  {
    eqtn->accept(*this);
    strings.push_back(ss.str());
    ss.str("");
  }
}

void EqtnPrinter::run(shared_ptr<Equation> eqtn)
{
  this->qualified = false;
  eqtn->accept(*this);
  strings.push_back(ss.str());
  ss.str("");
}

void EqtnPrinter::in(shared_ptr<Equation>)
{
  ss << " = ";
}
void EqtnPrinter::in(shared_ptr<Add>)
{
  ss << " + ";
}

void EqtnPrinter::in(shared_ptr<Subtract>)
{
  ss << " - ";
}

void EqtnPrinter::in(shared_ptr<Multiply>)
{
  ss << "*";
}

void EqtnPrinter::in(shared_ptr<Divide>)
{
  ss << "/";
}

void EqtnPrinter::in(shared_ptr<Symbol> sp)
{
  if(qualified)
    ss << elem->name->value << ".";
  ss << sp->value;
}

void EqtnPrinter::in(shared_ptr<Pow>)
{
  ss << "^";
}

void EqtnPrinter::in(shared_ptr<Real> rp)
{
  ss << rp->value;
}

void EqtnPrinter::in(shared_ptr<Differentiate>)
{
  ss << "'";
}

void EqtnPrinter::visit(shared_ptr<SubExpression>)
{
  ss << " (";
}

void EqtnPrinter::leave(shared_ptr<SubExpression>)
{
  ss << ") ";
}

