#include "Cypress/Control/ControlSystem.hxx"
#include <stdexcept>
#include <iostream>

using namespace cypress;
using namespace cypress::control;

void ControlSystem::emitSources()
{
  for(const ControlNode &cn: controlNodes)
    controlNodeSources.push_back(cn.emitSource());
}

void ControlSystem::buildControlNodes()
{
  for(ComponentSP cp: exp->components)
  {
    if(!(cp->element->kind() == Decl::Kind::Controller)) continue;
    ControlNode cn{cp->name->value};
    for(EquationSP eq: cp->element->eqtns) cn.eqtns.push_back(eq->clone());
    controlNodes.push_back(cn);
  }

  mapInputs();
  mapOutputs();
}

void ControlSystem::liftInput(ControlNode &cn, std::string vname)
{
  for(EquationSP eq: cn.eqtns)
  {
    VarLifter<IOVar> cvl(vname);
    eq->accept(cvl);
  }
}

void ControlSystem::mapInputs()
{
  for(ConnectionSP cx: exp->connections)
  {
    if(isa(cx->from, Connectable::Kind::SubComponent))
    {
      auto sc = std::static_pointer_cast<SubComponentRef>(cx->from);
      VarRefSP x = getControlled(sc);
      if(x->component->element->kind() == Decl::Kind::Controller)
      {
        auto it =
          find_if(controlNodes.begin(), controlNodes.end(),
              [x](const ControlNode &cn)
              {
                return cn.name == x->component->name->value;
              });

        if(it == controlNodes.end())
          throw std::runtime_error(
              "Undefined controller target `"+x->component->name->value+"`");

        ControlNode &tgt = *it;
        liftInput(tgt, sc->subname->value);
      }
    }
  }

}

void ControlSystem::mapOutputs()
{

}
  
