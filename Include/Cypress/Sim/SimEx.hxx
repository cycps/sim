#ifndef CYPRESS_SIMEX
#define CYPRESS_SIMEX

#include "Cypress/Sim/ComputeNode.hxx"

#include <string>
#include <vector>
#include <functional>

namespace cypress { namespace sim {

struct SimEx
{
  std::vector<ComputeNode> computeNodes;
  std::vector<std::string> computeNodeSources;

  void emitSources();
};

}} //::cypress::sim

#endif
