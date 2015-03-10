#ifndef CYPRESS_SIMEX
#define CYPRESS_SIMEX

#include "Cypress/Sim/ComputeNode.hxx"

#include <string>
#include <vector>
#include <functional>

namespace cypress { namespace sim {

struct SimEx
{
  size_t neq;
  std::vector<ComputeNode> computeNodes;
  std::vector<std::string> computeNodeSources;

  SimEx() = default;
  explicit SimEx(size_t neq);
};

}} //::cypress::sim

#endif
