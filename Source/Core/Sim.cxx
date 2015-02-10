#include "Cypress/Core/Sim.hxx"
#include <stdexcept>

using namespace cypress;
using std::shared_ptr;
using std::vector;
using std::runtime_error;
using std::make_shared;

Sim::Sim(
    vector<shared_ptr<Object>> objects,
    vector<shared_ptr<Controller>> controllers,
    shared_ptr<Experiment> exp) 
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

      for(auto p: c->params)
        applyParameter(cpy, c->name->value+"."+p.first->value, p.second->value);

      psys.push_back(cpy);
    }
  }
}

void Sim::buildPhysics()
{
  typeAssignComponents();
  buildSystemEquations();
}
  
std::shared_ptr<Element> 
Sim::findDecl(std::shared_ptr<Component> c)
{
  for(auto e : elements)
    if(c->kind->value == e->name->value) return e;

  return nullptr;
}

