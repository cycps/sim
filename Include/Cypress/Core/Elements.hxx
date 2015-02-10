#ifndef CYPRESS_ELEMENTS
#define CYPRESS_ELEMENTS

#include "Cypress/Core/Common.hxx"
#include "Cypress/Core/Equation.hxx"

#include <vector>
#include <unordered_map>

namespace cypress {

//Core data structures --------------------------------------------------------

struct Element : public Decl
{
  std::shared_ptr<Symbol> name;
  std::vector<std::shared_ptr<Symbol>> params;
  std::vector<std::shared_ptr<Equation>> eqtns; 
  Element(std::shared_ptr<Symbol> name) : name{name} {}
};

struct Object : public Element
{
  Kind kind() const override { return Kind::Object; }
  using Element::Element;
};

struct Controller : public Element
{
  Kind kind() const override { return Kind::Controller; }
  using Element::Element;
};

struct Component
{
  std::shared_ptr<Symbol> kind, name;
  std::unordered_map<std::shared_ptr<Symbol>, std::shared_ptr<Real>> params;
  std::shared_ptr<Element> element;
  Component(std::shared_ptr<Symbol> kind, std::shared_ptr<Symbol> name)
    : kind{kind}, name{name} {}
};

struct Linkable 
{
  enum class Kind { Thing, SubThing, AtoD };
  virtual Kind kind() const = 0;
};

struct Thing : public Linkable
{
  std::shared_ptr<Symbol> name;
  Kind kind() const override { return Kind::Thing; }
  Thing(std::shared_ptr<Symbol> name) : name{name} {}
};

struct SubThing : public Linkable
{
  std::shared_ptr<Symbol> name, subname;
  Kind kind() const override { return Kind::SubThing; }
  SubThing(std::shared_ptr<Symbol> name, std::shared_ptr<Symbol> subname)
    : name{name}, subname{subname} {}
};

struct AtoD : public Linkable
{
  double rate;
  Kind kind() const override { return Kind::AtoD; }
  AtoD(double rate) : rate{rate} {}
};

struct Link
{
  std::shared_ptr<Linkable> from, to;  
  Link(std::shared_ptr<Linkable> from, std::shared_ptr<Linkable> to)
    : from{from}, to{to} {}
};

struct Experiment : public Decl
{
  std::shared_ptr<Symbol> name;
  std::vector<std::shared_ptr<Component>> components;
  std::vector<std::shared_ptr<Link>> links;
  Kind kind() const override { return Kind::Experiment; }
  Experiment(std::shared_ptr<Symbol> name) : name{name} {}
};

struct Decls
{
  std::vector<std::shared_ptr<Object>> objects;
  std::vector<std::shared_ptr<Controller>> controllers;
  std::vector<std::shared_ptr<Experiment>> experiments;
};

//ostream operations ----------------------------------------------------------
std::ostream& operator << (std::ostream &o, const Decls &d);
std::ostream& operator << (std::ostream &o, const Object &obj);
std::ostream& operator << (std::ostream &o, const Controller &controller);
std::ostream& operator << (std::ostream &o, const Experiment &expr);
std::ostream& operator << (std::ostream &o, const Component &cp);
std::ostream& operator << (std::ostream &o, const Link &lnk);
std::ostream& operator << (std::ostream &o, const Linkable &lkb);
void showEqtn(std::ostream &, const Equation &);
void showExpr(size_t indent, std::ostream &o, const Expression &expr);

//Free functions over elements ------------------------------------------------
void
setEqtnsToZero(std::shared_ptr<Element>);

std::shared_ptr<Element>
qualifyEqtns(std::shared_ptr<Element>);

//Element Visitors ------------------------------------------------------------
struct EqtnQualifier : public Visitor
{
  std::shared_ptr<Component> qual{nullptr};
  void setQualifier(std::shared_ptr<Component>);
  void visit(std::shared_ptr<Symbol>) override;
  void run(std::shared_ptr<Equation>);
};

}

#endif
