#ifndef CYPRESS_EQUATION
#define CYPRESS_EQUATION

#include <string>
#include "Cypress/Core/Common.hxx"

//Forward Declarations --------------------------------------------------------
namespace cypress 
{
  struct Equation;      using EquationSP = std::shared_ptr<Equation>;
  struct Expression;    using ExpressionSP = std::shared_ptr<Expression>;
  struct Add;           using AddSP = std::shared_ptr<Add>;
  struct Subtract;      using SubtractSP = std::shared_ptr<Subtract>;
  struct Multiply;      using MultiplySP = std::shared_ptr<Multiply>;
  struct Divide;        using DivideSP = std::shared_ptr<Divide>;
  struct Pow;           using PowSP = std::shared_ptr<Pow>;
  struct Differentiate; using DifferentiateSP = std::shared_ptr<Differentiate>;
  struct Symbol;        using SymbolSP = std::shared_ptr<Symbol>;
  struct CVar;          using CVarSP = std::shared_ptr<CVar>;
  struct Real;          using RealSP = std::shared_ptr<Real>;
  struct SubExpression; using SubExpressionSP = std::shared_ptr<SubExpression>;
  struct Term;          using TermSP = std::shared_ptr<Term>;
  struct Factor;        using FactorSP = std::shared_ptr<Factor>;
  struct Atom;          using AtomSP = std::shared_ptr<Atom>;

}

namespace cypress 
{

//Core data structures --------------------------------------------------------
struct Expression : public ASTNode, public Clonable<Expression>
{
  enum class Kind{ 
    Add, Subtract,
    Multiply, Divide,
    Pow,
    Differentiate,
    Symbol, CVar,
    Real,
    SubExpression
  };
  virtual Kind kind() const = 0;
};

struct Term : public Expression {};

struct GroupOp : public Expression
{ 
  ExpressionSP lhs, rhs;
  GroupOp(ExpressionSP lhs, ExpressionSP rhs) : lhs{lhs}, rhs{rhs} {}
};

struct Add : public GroupOp, public std::enable_shared_from_this<Add> 
{ 
  Kind kind() const{ return Kind::Add; } 
  using GroupOp::GroupOp;
  void accept(Visitor &v) override;
  ExpressionSP clone() override;
};

struct Subtract : public GroupOp, public std::enable_shared_from_this<Subtract>
{
  Kind kind() const{ return Kind::Subtract; }
  using GroupOp::GroupOp;
  void accept(Visitor &v) override;
  ExpressionSP clone() override;
};

struct Factor : public Term {};

struct RingOp : public Term
{
  TermSP lhs, rhs;
  RingOp(TermSP lhs, TermSP rhs) : lhs{lhs}, rhs{rhs} {}
};

struct Multiply : public RingOp, public std::enable_shared_from_this<Multiply>
{
  Kind kind() const{ return Kind::Multiply; }
  using RingOp::RingOp;
  void accept(Visitor &v) override;
  ExpressionSP clone() override;
};

struct Divide : public RingOp, public std::enable_shared_from_this<Divide>
{
  Kind kind() const{ return Kind::Divide; }
  using RingOp::RingOp;
  void accept(Visitor &v) override;
  ExpressionSP clone() override;
};

struct Atom : public Factor {};

struct Pow : public Factor, public std::enable_shared_from_this<Pow>
{
  AtomSP lhs, rhs;
  Kind kind() const{ return Kind::Pow; }
  Pow(AtomSP lhs, AtomSP rhs) : lhs{lhs}, rhs{rhs} {}
  
  void accept(Visitor &v) override;
  ExpressionSP clone() override;
};

struct Symbol : public Atom, public std::enable_shared_from_this<Symbol>
{
  std::string value;
  Kind kind() const{ return Kind::Symbol; }
  Symbol(std::string value) : value{value} {}
  void accept(Visitor &v) override;
  ExpressionSP clone() override;
};

struct SymbolHash
{
  static const std::hash<std::string> hsh;
  size_t operator()(SymbolSP a);
};

struct SymbolEq
{
  SymbolHash sh{};
  bool operator()(SymbolSP a, SymbolSP b);
};

struct CVar : public Atom, public std::enable_shared_from_this<CVar>
{
  SymbolSP value;
  Kind kind() const{ return Kind::CVar; }
  CVar(SymbolSP value) : value{value} {}
  void accept(Visitor &v) override;
  ExpressionSP clone() override;
};

struct Differentiate : public Factor, 
                       public std::enable_shared_from_this<Differentiate>
{
  SymbolSP arg;
  Kind kind() const{ return Kind::Differentiate; }
  Differentiate(SymbolSP arg) : arg{arg} {}
  void accept(Visitor &v) override;
  ExpressionSP clone() override;
};

struct Real : public Atom, public std::enable_shared_from_this<Real>
{
  double value;
  Kind kind() const{ return Kind::Real; }
  Real(double value) : value{value} {}
  void accept(Visitor &v) override;
  ExpressionSP clone() override;
};

struct SubExpression : public Atom, 
                       public std::enable_shared_from_this<SubExpression>
{
  ExpressionSP value;
  SubExpression(ExpressionSP value) : value{value} {}
  Kind kind() const{ return Kind::SubExpression; }
  void accept(Visitor &v) override;
  ExpressionSP clone() override;
};

struct Decl
{
  enum class Kind { Object, Controller, Link, Experiment };
  virtual Kind kind() const = 0;
};

struct Equation : public ASTNode, 
                  public Clonable<Equation>,
                  public std::enable_shared_from_this<Equation>
{
  ExpressionSP lhs, rhs;

  void accept(Visitor &v) override;
  EquationSP clone() override;
};


//Equation Visitor ------------------------------------------------------------
struct Visitor
{
  virtual void visit(EquationSP) {}
  virtual void in(EquationSP) {}
  virtual void leave(EquationSP) {}

  virtual void visit(AddSP) {}
  virtual void in(AddSP) {}
  virtual void leave(AddSP) {}

  virtual void visit(SubtractSP) {}
  virtual void in(SubtractSP) {}
  virtual void leave(SubtractSP) {}

  virtual void visit(MultiplySP) {}
  virtual void in(MultiplySP) {}
  virtual void leave(MultiplySP) {}

  virtual void visit(DivideSP) {}
  virtual void in(DivideSP) {}
  virtual void leave(DivideSP) {}

  virtual void visit(PowSP) {}
  virtual void in(PowSP) {}
  virtual void leave(PowSP) {}

  virtual void visit(DifferentiateSP) {}
  virtual void in(DifferentiateSP) {}
  virtual void leave(DifferentiateSP) {}

  virtual void visit(CVarSP) {}
  virtual void in(CVarSP) {}
  virtual void leave(CVarSP) {}

  virtual void visit(SymbolSP) {}
  virtual void in(SymbolSP) {}
  virtual void leave(SymbolSP) {}
  
  virtual void visit(RealSP) {}
  virtual void in(RealSP) {}
  virtual void leave(RealSP) {}
  
  virtual void visit(SubExpressionSP) {}
  virtual void in(SubExpressionSP) {}
  virtual void leave(SubExpressionSP) {}
};

//Free functions over equations -----------------------------------------------
EquationSP
setToZero(EquationSP);

//Equation Visitors ===========================================================

//Equation parametrization ----------------------------------------------------
struct EqtnParametizer : public Visitor
{
  std::string symbol_name;
  double value;
  
  void visit(AddSP) override;
  void visit(SubtractSP) override;
  void visit(MultiplySP) override;
  void visit(DivideSP) override;
  void visit(PowSP) override;

  template<class BinOp>
  void apply(std::shared_ptr<BinOp>);

  template<class Kinded>
  void parametize(std::shared_ptr<Kinded>*);
};
void applyParameter(EquationSP, std::string symbol_name, double value);

//Controlled variable lifting -------------------------------------------------
struct CVarLifter : public Visitor
{
  std::string symbol_name;
  void visit(EquationSP) override;
  void visit(AddSP) override;
  void visit(SubtractSP) override;
  void visit(MultiplySP) override;
  void visit(DivideSP) override;
  void visit(PowSP) override;
  void visit(SubExpressionSP) override;

  template<class BinOp>
  void applyBinary(std::shared_ptr<BinOp>);

  template<class UnOp>
  void applyUnary(std::shared_ptr<UnOp>);

  template<class Kinded>
  void lift(std::shared_ptr<Kinded>*);
};
void liftControlledVars(EquationSP, std::string symbol_name);

} //::cypress
#include "Equation.hh"
#endif
