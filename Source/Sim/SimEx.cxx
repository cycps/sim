#include "Cypress/Sim/SimEx.hxx"
#include <iostream>
#include <sstream>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <vector>
#include <stdexcept>

using namespace::cypress;
using namespace::cypress::sim;
using std::cout;
using std::endl;
using std::string;
using std::stringstream;
using std::ifstream;
using std::vector;
using std::runtime_error;
using std::stol;
using std::stod;

void SimEx::emitSources()
{
  for(ComputeNode &c: computeNodes) 
    computeNodeSources.push_back(c.emitSource());
}
