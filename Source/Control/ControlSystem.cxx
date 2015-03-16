#include "Cypress/Control/ControlSystem.hxx"
#include <stdexcept>
#include <iostream>

using namespace cypress;
using namespace cypress::control;
using std::string;

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
    ControlNode cn{cp};
    for(EquationSP eq: cp->element->eqtns) cn.eqtns.push_back(eq->clone());
    for(BoundSP eq: cp->element->bounds) cn.bounds.push_back(eq->clone());
    controlNodes.push_back(cn);
  }

  mapInputs();
  mapOutputs();

  //note that this _must_ take place after mapping inputs to that the
  //inputs do not get included in the control compute space
  for(ControlNode &cn : controlNodes)
  {
    cn.extractComputeVars();
    cn.residualForm();
    cn.addInputResiduals();
    cn.applyBounds();
  }

}

void ControlSystem::liftInput(ControlNode &cn, std::string vname)
{
  for(EquationSP eq: cn.eqtns)
  {
    VarLifter<IOVar> cvl(vname);
    cvl.onlift = 
      [](IOVarSP x){ x->iokind = IOVar::IOKind::Input; };
    eq->accept(cvl);
  }
}

void ControlSystem::liftOutput(ControlNode &cn, std::string vname)
{
  for(EquationSP eq: cn.eqtns)
  {
    VarLifter<IOVar> cvl(vname);
    cvl.onlift = 
      [](IOVarSP x){ x->iokind = IOVar::IOKind::Output; };
    eq->accept(cvl);
  }
}

ControlNode& ControlSystem::controlNodeByName(string name)
{
  auto it =
    find_if(controlNodes.begin(), controlNodes.end(),
        [name](const ControlNode &cn)
        {
          return cn.name == name;
        });

  if(it == controlNodes.end())
    throw std::runtime_error(
        "Undefined controller target `"+name+"`");

  return *it;
}

void ControlSystem::mapInputs()
{
  for(ConnectionSP cx: exp->connections)
  {
    if(isa(cx->from, Connectable::Kind::SubComponent))
    {
      auto sc = std::static_pointer_cast<SubComponentRef>(cx->from);
      VarRefSP x = getDestination(sc);
      if(x->component->element->kind() == Decl::Kind::Controller)
      {

        ControlNode &tgt = controlNodeByName(x->component->name->value);
        tgt.inputs.push_back(
            {sc->subname->value, 
              {sc->name->value, sc->subname->value}});

        liftInput(tgt, sc->subname->value);
      }
    }
  }

}


void ControlSystem::mapOutputs()
{
  for(ConnectionSP cx: exp->connections)
  {
    if(isa(cx->from, Connectable::Kind::SubComponent))
    {
      auto sc = std::static_pointer_cast<SubComponentRef>(cx->from);
      if(isa(sc->component->element, Decl::Kind::Controller))
      {
        ControlNode &src = controlNodeByName(sc->name->value);
        VarRefSP x = getDestination(sc);

        src.outputs.push_back(
            {sc->subname->value,
             {x->component->name->value, x->name}});

        liftOutput(src, sc->subname->value);
      }
    }
  }
}
  
