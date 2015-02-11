#include "Cypress/Core/Sim.hxx"
#include <stdexcept>

using namespace cypress;
using std::vector;
using std::runtime_error;
using std::make_shared;
using std::string;
using std::static_pointer_cast;

Sim::Sim( vector<ObjectSP> objects, vector<ControllerSP> controllers,
    ExperimentSP exp) 
  : objects{objects}, controllers{controllers}, exp{exp}
{
  elements.insert(elements.end(), objects.begin(), objects.end());
  elements.insert(elements.end(), controllers.begin(), controllers.end());
}

/*
void Sim::typeAssignComponents()
{
  for(auto c : exp->components)
  {
    //TODO Need a better mechanisim for builtins
    if(c->kind->value == "Link") 
    {
      c->element = make_shared<Link>(c->name);
      //TODO: Assign parameter values to link
      continue;
    }

    auto cd = findDecl(c);  
    if(cd == nullptr) 
      throw runtime_error("Undefined Component Type: " + c->kind->value);
    c->element = cd;
  }
}
*/

void Sim::addObjectToSim(ComponentSP c)
{
  EqtnQualifier eqq;
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

#include <iostream>
void Sim::addControllerToSim(ComponentSP c)
{
  std::cout << "adding controller " 
            << c->name->value << " :: "
            << c->kind->value << std::endl;

  //vector<SubComponentRefSP> uc = findControlledSubComponents(c);  
}

void Sim::buildSystemEquations()
{
  for(auto c: exp->components)
  {
    if(c->element->kind() == Decl::Kind::Object) addObjectToSim(c);
    //if(c->element->kind() == Decl::Kind::Controller) addControllerToSim(c);
  }

  for(ConnectionSP cx : exp->links)
  {
    if(cx->from->kind() == Connectable::Kind::SubComponent)
    {
      auto sc = static_pointer_cast<SubComponentRef>(cx->from);
    }
  }
}

void Sim::buildPhysics()
{
  //typeAssignComponents();
  buildSystemEquations();
}
 
/*
ElementSP 
Sim::findDecl(ComponentSP c)
{
  for(auto e : elements)
    if(c->kind->value == e->name->value) return e;

  return nullptr;
}
*/
