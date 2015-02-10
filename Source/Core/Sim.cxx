#include "Cypress/Core/Sim.hxx"
#include <stdexcept>

using namespace cypress;
using std::vector;
using std::runtime_error;
using std::make_shared;
using std::string;

Sim::Sim( vector<ObjectSP> objects, vector<ControllerSP> controllers,
    ExperimentSP exp) 
  : objects{objects}, controllers{controllers}, exp{exp}
{
  elements.insert(elements.end(), objects.begin(), objects.end());
  elements.insert(elements.end(), controllers.begin(), controllers.end());
}

void Sim::typeAssignComponents()
{
  for(auto c : exp->components)
  {
    //TODO Need a better mechanisim for builtins
    if(c->kind->value == "Link") continue;
    auto cd = findDecl(c);  
    if(cd == nullptr) 
      throw runtime_error("Undefined Component Type: " + c->kind->value);
    c->element = cd;
  }
}

void Sim::buildSystemEquations()
{
  EqtnQualifier eqq;
  for(auto c: exp->components)
  {
    if(c->element == nullptr || 
       c->element->kind() != Decl::Kind::Object) continue;

    eqq.setQualifier(c);
    for(auto eqtn: c->element->eqtns)
    {
      auto cpy = eqtn->clone();
      eqq.run(cpy);
      setToZero(cpy);

      for(auto p: c->params)
      {
        string sym_name{c->name->value+"."+p.first->value};
        applyParameter(cpy, sym_name, p.second->value);
      }
        
      liftControlledVars(cpy, "rotor.ω");

      psys.push_back(cpy);
    }
  }
}

void Sim::buildPhysics()
{
  typeAssignComponents();
  buildSystemEquations();
}
  
ElementSP 
Sim::findDecl(ComponentSP c)
{
  for(auto e : elements)
    if(c->kind->value == e->name->value) return e;

  return nullptr;
}

