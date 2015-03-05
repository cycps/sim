#include "Cypress/Core/Var.hxx"
#include "Cypress/Core/Elements.hxx"

using namespace cypress;
using std::string;
  
VarRef::VarRef(ComponentSP component, string name)
  : component{component}, name{name}
{}

string VarRef::qname() const { return component->name->value + "_" + name; }
string DVarRef::qname() const { return component->name->value + "_d" + name; }

VarRef::Kind VarRef::kind() const { return Kind::Normal; }

VarRef::Kind DVarRef::kind() const { return Kind::Derivative; }


DVarRef::DVarRef(ComponentSP component, string name , size_t order)
  : VarRef{component, name}, order{order}
{

}
  
size_t VarRefSPHash::operator()(const VarRefSP v) const
{
  size_t result = 
    std::hash<int>{}(static_cast<int>(v->kind())) +
    std::hash<std::string>{}(v->name) +
    std::hash<std::string>{}(v->component->name->value);

  if(v->kind() == VarRef::Kind::Derivative)
  {
    auto dsp = std::static_pointer_cast<DVarRef>(v);
    result += std::hash<size_t>{}(dsp->order);
  }

  return result;
}

size_t VarRefSPNameHash::operator()(const VarRefSP v) const
{
  size_t result = 
    //std::hash<int>{}(static_cast<int>(v->kind())) +
    std::hash<std::string>{}(v->name) +
    std::hash<std::string>{}(v->component->name->value);

  /*
  if(v->kind() == VarRef::Kind::Derivative)
  {
    auto dsp = std::static_pointer_cast<DVarRef>(v);
    result += std::hash<size_t>{}(dsp->order);
  }
  */

  return result;
}
