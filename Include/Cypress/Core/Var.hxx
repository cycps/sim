#ifndef CYPRESS_VAR_HXX
#define CYPRESS_VAR_HXX

#include <string>
#include <functional>
#include <memory>
#include "Cypress/Sim/Resolve.hxx"
#include "Cypress/Core/Elements.hxx"

//Forward Declarations --------------------------------------------------------
namespace cypress
{
  struct VarRef; using VarRefSP = std::shared_ptr<VarRef>;
  struct DVarRev; using DVarRefSP = std::shared_ptr<DVarRev>;
}

namespace cypress
{

struct Initials
{
  double v{0}, d{0};
};

struct VarRef
{
  VarRef(ComponentSP component, SymbolSP name);

  enum class Kind { Normal, Derivative };
  virtual Kind kind() const;

  ComponentSP component;
  SymbolSP name;
  
  std::string qname() const;
};

struct DVarRef : public VarRef
{
  using VarRef::VarRef;

  Kind kind() const override;
  size_t order;
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
