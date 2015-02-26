#ifndef CYPRESS_SIM_COMPUTENODE
#define CYPRESS_SIM_COMPUTENODE

#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>
#include "Cypress/Sim/Var.hxx"
#include "Cypress/Sim/Resolve.hxx"
#include "Cypress/Core/Equation.hxx"

namespace cypress
{

struct ComputeNode
{
  size_t id;
  std::string expInfo;
  std::vector<std::string> vars;
  std::vector<RVar> rvars;  
  std::vector<EquationSP> eqtns;
  std::unordered_map<size_t, Initials> initials;

  std::string emitSource();
};

std::ostream & operator << (std::ostream &, const ComputeNode &);

}

#endif
