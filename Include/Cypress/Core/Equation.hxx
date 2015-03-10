#ifndef CYPRESS_EQUATION
#define CYPRESS_EQUATION

#include "Cypress/Core/Common.hxx"
#include <string>
#include <unordered_set>

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
  struct CCVar;         using CCVarSP = std::shared_ptr<CCVar>;
  struct BoundVar;      using BoundVarSP = std::shared_ptr<BoundVar>;
  struct Real;          using RealSP = std::shared_ptr<Real>;
  struct SubExpression; using SubExpressionSP = std::shared_ptr<SubExpression>;
  struct Term;          using TermSP = std::shared_ptr<Term>;
  struct Factor;        using FactorSP = std::shared_ptr<Factor>;
  struct Atom;          using AtomSP = std::shared_ptr<Atom>;
  struct VarType;       using VarTypeSP = std::shared_ptr<VarType>;
  struct Bound;         using BoundSP = std::shared_ptr<Bound>;
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
    Symbol, CVar, CCVar, BoundVar,
    Real,
    SubExpression
  };
  virtual Kind kind() const = 0;
  using ASTNode::ASTNode;
};

struct Term : public Expression 
{
  using Expression::Expression;
};

struct GroupOp : public Expression
{ 
  ExpressionSP lhs, rhs;
  GroupOp(ExpressionSP lhs, ExpressionSP rhs, size_t line, size_t column) 
    : Expression{line, column}, lhs{lhs}, rhs{rhs} {}
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

struct Factor : public Term 
{
  using Term::Term;
};

struct RingOp : public Term
{
  TermSP lhs, rhs;
  RingOp(TermSP lhs, TermSP rhs, size_t line, size_t column) 
    : Term{line, column}, lhs{lhs}, rhs{rhs} {}
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

struct Atom : public Factor 
{
  using Factor::Factor;
};

struct Pow : public Factor, public std::enable_shared_from_this<Pow>
{
  AtomSP lhs, rhs;
  Kind kind() const{ return Kind::Pow; }
  Pow(AtomSP lhs, AtomSP rhs, size_t line, size_t column) 
    : Factor{line, column}, lhs{lhs}, rhs{rhs} {}
  
  void accept(Visitor &v) override;
  ExpressionSP clone() override;
};

struct VarType : public Atom
{
  using Atom::Atom;
};

struct Symbol : public VarType, public std::enable_shared_from_this<Symbol>
{
  std::string value;
  Kind kind() const{ return Kind::Symbol; }
  Symbol(std::string value, size_t line, size_t column) 
    : VarType{line, column}, value{value} {}
  void accept(Visitor &v) override;
  ExpressionSP clone() override;
};

struct SymbolHash
{
  size_t operator()(SymbolSP a);
};

struct SymbolEq
{
  SymbolHash sh{};
  bool operator()(SymbolSP a, SymbolSP b);
};

struct CVar : public Atom, public std::enable_shared_from_this<CVar>
{
  VarTypeSP value;
  Kind kind() const{ return Kind::CVar; }
  CVar(VarTypeSP value) : Atom{value->line, value->column}, value{value} {}
  void accept(Visitor &v) override;
  ExpressionSP clone() override;
};

struct CCVar : public Atom, public std::enable_shared_from_this<CCVar>
{
  SymbolSP value;
  Kind kind() const{ return Kind::CCVar; }
  CCVar(SymbolSP value) : Atom{value->line, value->column}, value{value} {}
  void accept(Visitor &v) override;
  ExpressionSP clone() override;
};

struct BoundVar : public Atom, public std::enable_shared_from_this<BoundVar>
{
  VarTypeSP value;
  BoundSP bound;
  Kind kind() const{ return Kind::BoundVar; }
  BoundVar(VarTypeSP value) : Atom{value->line, value->column}, value{value} {}
  void accept(Visitor &v) override;
  ExpressionSP clone() override;
};

struct Differentiate : public VarType, 
                       public std::enable_shared_from_this<Differentiate>
{
  SymbolSP arg;
  Kind kind() const{ return Kind::Differentiate; }
  Differentiate(SymbolSP arg, size_t line, size_t column) 
    : VarType{line, column}, arg{arg} {}
  void accept(Visitor &v) override;
  ExpressionSP clone() override;
};

struct Real : public Atom, public std::enable_shared_from_this<Real>
{
  double value;
  Kind kind() const{ return Kind::Real; }
  Real(double value, size_t line, size_t column) 
    : Atom{line, column}, value{value} {}
  void accept(Visitor &v) override;
  ExpressionSP clone() override;
};

struct SubExpression : public Atom, 
                       public std::enable_shared_from_this<SubExpression>
{
  ExpressionSP value;
  SubExpression(ExpressionSP value) 
    : Atom{value->line, value->column}, value{value} {}
  Kind kind() const{ return Kind::SubExpression; }
  void accept(Visitor &v) override;
  ExpressionSP clone() override;
};

struct Decl : public Lexeme
{
  enum class Kind { Object, Controller, Link, Experiment };
  virtual Kind kind() const = 0;
  using Lexeme::Lexeme;
};

struct Equation : public ASTNode, 
                  public Clonable<Equation>,
                  public std::enable_shared_from_this<Equation>
{
  ExpressionSP lhs{nullptr}, rhs{nullptr};
  void accept(Visitor &v) override;
  EquationSP clone() override;
  using ASTNode::ASTNode;
};

struct Bound : public Lexeme
{
  enum class Kind { LT, AbsLT, GT, AbsGT };
  Kind kind;
  AtomSP lhs{nullptr}, rhs{nullptr};
  using Lexeme::Lexeme;
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
  
  virtual void visit(CCVarSP) {}
  virtual void in(CCVarSP) {}
  virtual void leave(CCVarSP) {}
  
  virtual void visit(BoundVarSP) {}
  virtual void in(BoundVarSP) {}
  virtual void leave(BoundVarSP) {}

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
template<class Lifter>
struct VarLifter : public Visitor
{
  bool lifts_derivs;
  std::string symbol_name;
  VarLifter(bool lifts_derivs, std::string symbol_name)
    : lifts_derivs{lifts_derivs}, symbol_name{symbol_name}
  {}
  void visit(EquationSP) override;
  void visit(AddSP) override;
  void visit(SubtractSP) override;
  void visit(MultiplySP) override;
  void visit(DivideSP) override;
  void visit(PowSP) override;
  void visit(SubExpressionSP) override;

  template<class BinOp>
  void liftBinary(std::shared_ptr<BinOp>, std::string symbol_name);

  template<class UnOp>
  void liftUnary(std::shared_ptr<UnOp>, std::string symbol_name);

  template<class Kinded>
  void lift(std::shared_ptr<Kinded>*, std::string symbol_name);
};
  

void liftControlledVars(EquationSP, std::string symbol_name);

} //::cypress
#include "Equation.hh"
#endif
