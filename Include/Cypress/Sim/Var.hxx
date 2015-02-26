#ifndef CYPRESS_VAR_HXX
#define CYPRESS_VAR_HXX

#include <string>
#include <functional>
#include "Cypress/Sim/Resolve.hxx"

namespace cypress
{

struct Initials
{
  double v{0}, d{0};
};

struct MetaVar
{
  std::string name;
  bool derivative, controlled;
  double initial{0};
  MetaVar()
    : derivative{false}, controlled{false}
  {}

  MetaVar(bool deriv, bool ctrl)
    : derivative{deriv}, controlled{ctrl}
  {}

  MetaVar(std::string name, bool deriv, bool ctrl)
    : name{name}, derivative{deriv}, controlled{ctrl}
  {}
};

struct MetaVarHash
{
  size_t operator()(const MetaVar &v)
  {
    return 
      std::hash<std::string>{}(v.name) + 
      std::hash<bool>{}(v.derivative) +
      std::hash<bool>{}(v.controlled);
  }
};

struct MetaVarCmp
{
  bool operator()(const MetaVar &a, const MetaVar &b)
  {
    return 
      std::hash<std::string>{}(a.name) == 
        std::hash<std::string>{}(b.name) &&
      a.derivative == b.derivative &&
      a.controlled == b.controlled;
  }
};

}

#endif
