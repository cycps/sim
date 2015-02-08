#include "Cypress/Compiler/Sema.hxx"

using namespace cypress;
using namespace cypress::compile;
using std::find_if;

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

   vars.insert(s); 
}

void VarCollector::leave(shared_ptr<Differentiate> s)
{
  dblock = false;
}

void VarCollector::visit(shared_ptr<Differentiate> s)
{
  derivs.insert(s->arg);
  dblock = true;
}

// Eqtn Printer ---------------------------------------------------------------
void EqtnPrinter::in(shared_ptr<Equation> ep)
{
  std::cout << " = ";
}
void EqtnPrinter::in(shared_ptr<Add> ap)
{
  std::cout << " + ";
}

void EqtnPrinter::in(shared_ptr<Subtract> sp)
{
  std::cout << " - ";
}

void EqtnPrinter::in(shared_ptr<Multiply> mp)
{
  std::cout << "*";
}

void EqtnPrinter::in(shared_ptr<Divide> dp)
{
  std::cout << "/";
}

void EqtnPrinter::in(shared_ptr<Symbol> sp)
{
  std::cout << sp->value;
}

void EqtnPrinter::in(shared_ptr<Pow> sp)
{
  std::cout << "^";
}

void EqtnPrinter::in(shared_ptr<Real> rp)
{
  std::cout << rp->value;
}

void EqtnPrinter::in(shared_ptr<Differentiate> dp)
{
  std::cout << "'";
}

void EqtnPrinter::visit(shared_ptr<SubExpression> sp)
{
  std::cout << " (";
}

void EqtnPrinter::leave(shared_ptr<SubExpression> sp)
{
  std::cout << ") ";
}
