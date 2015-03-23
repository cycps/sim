#ifndef CYPRESS_SIM_COMPUTENODE
#define CYPRESS_SIM_COMPUTENODE

#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>
#include "Cypress/Core/Var.hxx"
#include "Cypress/Sim/Resolve.hxx"
#include "Cypress/Core/Equation.hxx"
#include "Cypress/Core/Elements.hxx"

namespace cypress { namespace sim {

struct ComputeNode
{
  size_t id{0}, N{0}, cN{0};
  std::string expInfo;
  std::vector<VarRefSP> vars;
  std::vector<RVar> rvars;  
  std::unordered_multimap<ComponentSP, EquationSP> eqtns;
  std::unordered_map<size_t, Initials> initials;
  std::vector<SensorAttributesSP> sensors;
  std::vector<ActuatorAttributesSP> actuators;
  size_t varidx(VarRefSP);

  std::string emitSource();
};

std::ostream & operator << (std::ostream &, const ComputeNode &);

}} //::cypress::sim

#endif
