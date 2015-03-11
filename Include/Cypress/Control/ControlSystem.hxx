#ifndef CYPRESS_CONTROL_CONTROLSYSTEM
#define CYPRESS_CONTROL_CONTROLSYSTEM

#include "Cypress/Control/ControlNode.hxx"
#include "Cypress/Core/Elements.hxx"
#include <vector>
#include <memory>

namespace cypress { namespace control {

struct ControlSystem; using ControlSystemSP = std::shared_ptr<ControlSystem>;

struct ControlSystem
{
  std::vector<ControllerSP> controllerDefs;
  ExperimentSP exp;

  std::vector<ControlNode> controlNodes;
  std::vector<std::string> controlNodeSources;

  ControlSystem(std::vector<ControllerSP> cdefs, ExperimentSP exp)
    : controllerDefs{cdefs}, exp{exp}
  {}

  void buildControlNodes();
  void mapInputs();
  void mapOutputs();
  void emitSources();
  void liftInput(ControlNode &, std::string vname);
  void liftOutput(ControlNode &, std::string vname);
  ControlNode& controlNodeByName(std::string name);
};

}}

#endif
