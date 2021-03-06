#ifndef CYPRESS_ELEMENTS
#define CYPRESS_ELEMENTS

#include "Cypress/Core/Common.hxx"
#include "Cypress/Core/Equation.hxx"
#include "Cypress/Core/Var.hxx"

#include <vector>
#include <unordered_map>
#include <sstream>

//Forward Declarations --------------------------------------------------------
namespace cypress
{
  struct Element; using ElementSP = std::shared_ptr<Element>;
  struct Object; using ObjectSP = std::shared_ptr<Object>;
  struct Controller; using ControllerSP = std::shared_ptr<Controller>;
  struct Link; using LinkSP = std::shared_ptr<Link>;
  struct Actuator; using ActuatorSP = std::shared_ptr<Actuator>;
  struct Sensor; using SensorSP = std::shared_ptr<Sensor>;
  struct Component; using ComponentSP = std::shared_ptr<Component>;
  struct ComponentAttributes; 
  using ComponentAttributesSP = std::shared_ptr<ComponentAttributes>;
  struct SensorAttributes;
  using SensorAttributesSP = std::shared_ptr<SensorAttributes>;
  struct ActuatorAttributes;
  using ActuatorAttributesSP = std::shared_ptr<ActuatorAttributes>;
  struct Connectable; using ConnectableSP = std::shared_ptr<Connectable>;
  struct ComponentRef; using ComponentRefSP = std::shared_ptr<ComponentRef>;
  struct SubComponentRef; using SubComponentRefSP = 
    std::shared_ptr<SubComponentRef>;
  struct AtoD; using AtoDSP = std::shared_ptr<AtoD>;
  struct Connection; using ConnectionSP = std::shared_ptr<Connection>;
  struct Experiment; using ExperimentSP = std::shared_ptr<Experiment>;
  using Simulation = Experiment;
  using SimulationSP = ExperimentSP;
  
  struct VarRef; using VarRefSP = std::shared_ptr<VarRef>;
  struct VarRefSPHash;
  struct VarRefSPCmp;
}
namespace cypress 
{

//Core data structures --------------------------------------------------------

struct Element : public Decl
{
  SymbolSP name;
  std::vector<SymbolSP> params;
  std::vector<EquationSP> eqtns; 
  std::vector<BoundSP> bounds;
  Element(SymbolSP name, size_t line, size_t column) 
    : Decl{line, column}, name{name} {}
};

struct Object : public Element
{
  /*
  std::vector<SymbolSP> inputs;
  std::unordered_map<VarRefSP, ActuatorSP, VarRefSPHash, VarRefSPCmp> 
    actuators;
    */

  Kind kind() const override { return Kind::Object; }
  using Element::Element;
};

struct Controller : public Element
{
  Kind kind() const override { return Kind::Controller; }
  using Element::Element;
};

struct Link : public Element
{
  Kind kind() const override { return Kind::Link; }
  Link(SymbolSP name, size_t line, size_t column);
};

struct Actuator : public Element
{
  Kind kind() const override { return Kind::Actuator; }
  Actuator(SymbolSP name, size_t line, size_t colunn);
};

struct Sensor : public Element
{
  Kind kind() const override { return Kind::Sensor; }
  Sensor(SymbolSP name, size_t line, size_t column);
};

struct ComponentAttributes {};

struct SensorAttributes : public ComponentAttributes
{
  VarRefSP target{nullptr};
  size_t rate{0};
  std::string destination;
};

struct ActuatorAttributes : public ComponentAttributes
{
  VarRefSP source{nullptr}, target{nullptr};
  double min, max, dmin, dmax;
};

struct Component : public Lexeme
{
  SymbolSP kind, name;
  //std::unordered_map<SymbolSP, RealSP> params;
  std::unordered_map<SymbolSP, std::string> params;
  std::unordered_map<VarRefSP, RealSP, VarRefSPHash, VarRefSPCmp> initials;
  ElementSP element;
  Component(SymbolSP kind, SymbolSP name, size_t line, size_t column) 
    : Lexeme{line, column}, kind{kind}, name{name} {}

  std::string parameterValue(std::string);
  SymbolSP parameter(std::string);
  RealSP initialValue(std::string, VarRef::Kind k = VarRef::Kind::Normal);

  ComponentAttributesSP attributes;
  
  void applyParameters();
};


struct Connectable 
{
  enum class Kind { Component, SubComponent, AtoD };
  //only 1 neighbor for now
  //std::vector<ConnectableSP> neighbors;
  Connectable(SymbolSP name) : name{name} {}
  SymbolSP name;
  ConnectableSP neighbor{nullptr};
  virtual Kind kind() const = 0;
};

struct ComponentRef : public Connectable
{
  ComponentSP component{nullptr};
  Kind kind() const override { return Kind::Component; }
  //ComponentRef(SymbolSP name) : name{name} {}
  using Connectable::Connectable;
};

struct SubComponentRef : public ComponentRef
{
  SymbolSP subname;
  Kind kind() const override { return Kind::SubComponent; }
  SubComponentRef(SymbolSP name, SymbolSP subname) 
    : ComponentRef{name}, subname{subname} {}
};

struct AtoD : public Connectable
{
  RealSP rate;
  Kind kind() const override { return Kind::AtoD; }
  AtoD(RealSP rate) 
    : Connectable{std::make_shared<Symbol>(
        "balderdash", rate->line, rate->column)},
      rate{rate} 
  {}
};

struct Connection
{
  ConnectableSP from, to;  
  Connection(ConnectableSP from, ConnectableSP to) : from{from}, to{to} {}
};

struct Experiment : public Decl
{
  SymbolSP name;
  std::vector<ComponentSP> components;
  std::vector<ConnectionSP> connections;
  Kind kind() const override { return Kind::Experiment; }
  Experiment(SymbolSP name, size_t line, size_t column) 
    : Decl{line, column}, name{name} {}

  ComponentSP operator[](std::string);
};

struct Decls
{
  std::vector<ObjectSP> objects;
  std::vector<ControllerSP> controllers;
  std::vector<LinkSP> links;
  std::vector<ExperimentSP> experiments;
  Decls & operator += (const Decls &b);
};


//ostream operations ----------------------------------------------------------
std::ostream& operator << (std::ostream &o, const Decls &d);
std::ostream& operator << (std::ostream &o, const Object &obj);
std::ostream& operator << (std::ostream &o, const Controller &controller);
std::ostream& operator << (std::ostream &o, const Experiment &expr);
std::ostream& operator << (std::ostream &o, const Component &cp);
std::ostream& operator << (std::ostream &o, const Connection &lnk);
std::ostream& operator << (std::ostream &o, const Connectable &lkb);
void showEqtn(std::ostream &, const Equation &);
void showExpr(size_t indent, std::ostream &o, const Expression &expr);

//Free functions over elements ------------------------------------------------
void
setEqtnsToZero(ElementSP);

ElementSP
qualifyEqtns(ElementSP);

VarRefSP getDestination(ConnectableSP c);

//Element Visitors ------------------------------------------------------------
struct EqtnQualifier : public Visitor
{
  ComponentSP qual{nullptr};
  void setQualifier(ComponentSP);
  void visit(SymbolSP) override;
  void run(EquationSP);
};

struct EqtnPrinter : public Visitor
{
  std::shared_ptr<Element> elem;
  void run(std::shared_ptr<Element> e, bool qualified=false);
  void run(std::shared_ptr<Equation> eqtn);
  bool qualified{false};
  std::stringstream ss;
  std::vector<std::string> strings;

  void in(std::shared_ptr<Equation>) override;
  void in(std::shared_ptr<Add>) override;
  void in(std::shared_ptr<Subtract>) override;
  void in(std::shared_ptr<Multiply>) override;
  void in(std::shared_ptr<Divide>) override;
  void in(std::shared_ptr<Symbol>) override;
  void in(std::shared_ptr<Pow>) override;
  void in(std::shared_ptr<Real>) override;
  void in(std::shared_ptr<Differentiate>) override;
  void visit(std::shared_ptr<SubExpression>) override;
  void leave(std::shared_ptr<SubExpression>) override;
  void visit(CVarSP) override;
  void leave(CVarSP) override;
  void visit(CCVarSP) override;
  void leave(CCVarSP) override;
  void visit(BoundVarSP) override;
  void leave(BoundVarSP) override;
  void visit(IOVarSP) override;
  void leave(IOVarSP) override;
};

struct CxxResidualFuncBuilder : public Visitor
{
  std::stringstream ss;
  bool qnames{true};

  std::string run(ComponentSP cp, EquationSP, size_t idx);
  ComponentSP cp;

  void in(AddSP) override;
  void in(SubtractSP) override;
  void in(MultiplySP) override;
  void in(DivideSP) override;
  void in(SymbolSP) override;
  void visit(PowSP) override;
  void in(PowSP) override;
  void leave(PowSP) override;
  void in(RealSP) override;
  void visit(DifferentiateSP) override;
  void visit(SubExpressionSP) override;
  void leave(SubExpressionSP) override;
  void visit(CCVarSP) override;
  //void leave(CCVarSP) override;
  void visit(IOVarSP) override;
  void visit(BoundVarSP) override;
  void leave(BoundVarSP) override;
};

//Variable extraction ----------------------------------------------

struct VarContext 
{
  bool cvar{false}, deriv{false}, input{false}, output{false};
};

struct VarExtractor : Visitor
{
  ComponentSP component;

  //filter(symbol, inCVar, inDeriv)
  std::function<bool(SymbolSP, VarContext)> filter;

  VarContext ctx;
  //bool inCVar{false}, inDeriv{false}, inInput{false}, inOutput{false};
  std::unordered_set<VarRefSP, VarRefSPHash, VarRefSPCmp>
    vars;

  void run(ComponentSP, EquationSP);
  void run(ComponentSP);

  VarExtractor(std::function<bool(SymbolSP, VarContext)> f)
    : filter{f}
  {}

  private:
    void visit(DifferentiateSP) override;
    void leave(DifferentiateSP) override;
    void visit(CVarSP) override;
    void leave(CVarSP) override;
    void visit(IOVarSP) override;
    void leave(IOVarSP) override;
    void in(SymbolSP) override;
};

struct VarExtractorFactory
{
  static VarExtractor CVarExtractor()
  {
    return 
      VarExtractor(
        [](SymbolSP, VarContext ctx)
        {
          return ctx.cvar;
        });
  }
  
  static VarExtractor NonCVarExtractor()
  {
    return 
      VarExtractor(
        [](SymbolSP, VarContext ctx)
        {
          return !ctx.cvar;
        });
  }
  
  static VarExtractor AnyVarExtractor()
  {
    return 
      VarExtractor(
        [](SymbolSP, VarContext)
        {
          return true;
        });
  }

  static VarExtractor NonDerivExtractor()
  {
    return
      VarExtractor(
          [](SymbolSP, VarContext ctx)
          {
            return !ctx.deriv;
          });
  }
  
  static VarExtractor DerivExtractor()
  {
    return
      VarExtractor(
          [](SymbolSP, VarContext ctx)
          {
            return ctx.deriv;
          });
  }
};

}

#endif
