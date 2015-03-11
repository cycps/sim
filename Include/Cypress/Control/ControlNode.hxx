#ifndef CYPRESS_CONTROL_CONTROLNODE
#define CYPRESS_CONTROL_CONTROLNODE

#include "Cypress/Core/Equation.hxx"

#include <string>
#include <vector>

namespace cypress { namespace control {

//fully qualified control variable
struct FQCV 
{ 
  FQCV(std::string who, std::string what) : who{who}, what{what} {}
  std::string who, what; 
};

//map for RX/TX from controllers
struct IOMap 
{ 
  IOMap(std::string local, FQCV remote) : local{local}, remote{remote} {}
  std::string local; FQCV remote; 
};

struct ControlNode
{
  std::string name;
  std::vector<IOMap> inputs, outputs;
  std::vector<EquationSP> eqtns;

  ControlNode() = default;
  explicit ControlNode(std::string name) : name{name} {}
  std::string emitSource() const;
};

std::ostream & operator << (std::ostream &, const ControlNode &);

}}

#endif
