#ifndef CYPRESS_SIMEX
#define CYPRESS_SIMEX

#include "Cypress/Sim/ComputeNode.hxx"

#include <string>
#include <vector>
#include <functional>

namespace cypress
{

struct SimEx
{
  size_t neq;
  double rtol, satol;

  std::vector<ComputeNode> computeNodes;
  std::vector<std::string> computeNodeSources;

  std::string residualClosureSource;

  SimEx() = default;
  explicit SimEx(size_t neq, double rtol, double satol);
  explicit SimEx(std::string source);
  void run();
  
  void startupReport();

  std::string toString();

  size_t parseNEQ(std::string ln);
  double parseRTOL(std::string ln);
  double parseSATOL(std::string ln);

};

}

#endif
