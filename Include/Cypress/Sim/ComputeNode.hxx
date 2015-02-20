#ifndef CYPRESS_SIM_COMPUTENODE
#define CYPRESS_SIM_COMPUTENODE

#include <vector>
#include <string>
#include <iostream>
#include "Cypress/Sim/Resolve.hxx"
#include "Cypress/Core/Equation.hxx"

namespace cypress
{

struct ComputeNode
{
  size_t id;
  std::vector<std::string> vars;
  std::vector<RVar> rvars;  
  std::vector<EquationSP> eqtns;
};

std::ostream & operator << (std::ostream &, const ComputeNode &);

}

#endif
