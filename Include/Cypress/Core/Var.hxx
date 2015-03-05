#ifndef CYPRESS_VAR_HXX
#define CYPRESS_VAR_HXX

#include <string>
#include <functional>
#include <memory>
//#include "Cypress/Sim/Resolve.hxx"
//#include "Cypress/Core/Elements.hxx"

//Forward Declarations --------------------------------------------------------
namespace cypress
{
  struct VarRef; using VarRefSP = std::shared_ptr<VarRef>;
  struct DVarRev; using DVarRefSP = std::shared_ptr<DVarRev>;
  struct Component; using ComponentSP = std::shared_ptr<Component>;
  struct VarRefSPHash;
  struct VarRefSPCmp;
}

namespace cypress
{

struct Initials
{
  double v{0}, d{0};
};

struct VarRef
{
  VarRef(ComponentSP, std::string);

  enum class Kind { Normal, Derivative };
  virtual Kind kind() const;

  ComponentSP component;
  std::string name;
  bool controlled{false};
  
  virtual std::string qname() const;
};

struct DVarRef : public VarRef
{
  using VarRef::VarRef;
  DVarRef(ComponentSP, std::string, size_t);

  Kind kind() const override;
  std::string qname() const override;
  size_t order;
};

struct VarRefSPHash
{
  size_t operator()(const VarRefSP v) const;
};

struct VarRefSPCmp
{
  bool operator()(const VarRefSP a, const VarRefSP b) const
  {
    VarRefSPHash hsh{};
    return hsh(a) == hsh(b);
  }
};

struct VarRefSPNameHash
{
  size_t operator()(const VarRefSP v) const;
};

struct VarRefSPNameCmp
{
  bool operator()(const VarRefSP a, const VarRefSP b) const
  {
    VarRefSPNameHash hsh{};
    return hsh(a) == hsh(b);
  }
};

}

#endif
