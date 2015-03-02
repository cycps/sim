#include "Cypress/Core/Var.hxx"

using namespace cypress;
using std::string;
  
VarRef::VarRef(ComponentSP component, SymbolSP name)
  : component{component}, name{name}
{}

string VarRef::qname() const { return component->name->value + "_" + name->value; }

VarRef::Kind VarRef::kind() const { return Kind::Normal; }

VarRef::Kind DVarRef::kind() const { return Kind::Derivative; }

