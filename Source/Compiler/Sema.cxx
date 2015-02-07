#include "Cypress/Compiler/Sema.hxx"

using namespace cypress;
using namespace cypress::compile;

using std::shared_ptr;

void VarCollector::visit(shared_ptr<Symbol> s)
{
   if(!dblock) vars.insert(s); 
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
