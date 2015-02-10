#ifndef CYPRESS_EQUATION
#define CYPRESS_EQUATION

#include <string>
#include "Cypress/Core/Common.hxx"

//Forward Declarations --------------------------------------------------------
namespace cypress 
{
  struct Equation;
  struct Expression;
  struct Add;
  struct Subtract;
  struct Multiply;
  struct Divide;
  struct Pow;
  struct Differentiate;
  struct Symbol;
  struct Real;
  struct SubExpression;

}

namespace cypress {

//Core data structures --------------------------------------------------------
struct Expression : public ASTNode, public Clonable<Expression>
{
  enum class Kind{ 
    Add, Subtract,
    Multiply, Divide,
    Pow,
    Differentiate,
    Symbol,
    Real,
    SubExpression
  };
  virtual Kind kind() const = 0;
};

struct Term : public Expression {};

struct GroupOp : public Expression
{ 
  std::shared_ptr<Expression> lhs, rhs;
  GroupOp(std::shared_ptr<Expression> lhs, std::shared_ptr<Expression> rhs) 
    : lhs{lhs}, rhs{rhs} {}
};

struct Add : public GroupOp, public std::enable_shared_from_this<Add> 
{ 
  Kind kind() const{ return Kind::Add; } 
  using GroupOp::GroupOp;
  void accept(Visitor &v) override;
  std::shared_ptr<Expression> clone() override;
};

struct Subtract : public GroupOp, public std::enable_shared_from_this<Subtract>
{
  Kind kind() const{ return Kind::Subtract; }
  using GroupOp::GroupOp;
  void accept(Visitor &v) override;
  std::shared_ptr<Expression> clone() override;
};

struct Factor : public Term {};

struct RingOp : public Term
{
  std::shared_ptr<Term> lhs, rhs;
  RingOp(std::shared_ptr<Term> lhs, std::shared_ptr<Term> rhs)
    : lhs{lhs}, rhs{rhs} {}
};

struct Multiply : public RingOp, public std::enable_shared_from_this<Multiply>
{
  Kind kind() const{ return Kind::Multiply; }
  using RingOp::RingOp;
  void accept(Visitor &v) override;
  std::shared_ptr<Expression> clone() override;
};

struct Divide : public RingOp, public std::enable_shared_from_this<Divide>
{
  Kind kind() const{ return Kind::Divide; }
  using RingOp::RingOp;
  void accept(Visitor &v) override;
  std::shared_ptr<Expression> clone() override;
};

struct Atom : public Factor {};

struct Pow : public Factor, public std::enable_shared_from_this<Pow>
{
  std::shared_ptr<Atom> lhs, rhs;
  Kind kind() const{ return Kind::Pow; }
  Pow(std::shared_ptr<Atom> lhs, std::shared_ptr<Atom> rhs)
    : lhs{lhs}, rhs{rhs} {}
  
  void accept(Visitor &v) override;
  std::shared_ptr<Expression> clone() override;
};

struct Symbol : public Atom, public std::enable_shared_from_this<Symbol>
{
  std::string value;
  Kind kind() const{ return Kind::Symbol; }
  Symbol(std::string value) : value{value} {}
  void accept(Visitor &v) override;
  std::shared_ptr<Expression> clone() override;
};

struct SymbolHash
{
  static const std::hash<std::string> hsh;
  size_t operator()(std::shared_ptr<Symbol> a);
};

struct SymbolEq
{
  SymbolHash sh{};
  bool operator()(std::shared_ptr<Symbol> a, std::shared_ptr<Symbol> b);
};

struct Differentiate : public Factor, 
                       public std::enable_shared_from_this<Differentiate>
{
  std::shared_ptr<Symbol> arg;
  Kind kind() const{ return Kind::Differentiate; }
  Differentiate(std::shared_ptr<Symbol> arg) : arg{arg} {}
  void accept(Visitor &v) override;
  std::shared_ptr<Expression> clone() override;
};

struct Real : public Atom, public std::enable_shared_from_this<Real>
{
  double value;
  Kind kind() const{ return Kind::Real; }
  Real(double value) : value{value} {}
  void accept(Visitor &v) override;
  std::shared_ptr<Expression> clone() override;
};

struct SubExpression : public Atom, 
                       public std::enable_shared_from_this<SubExpression>
{
  std::shared_ptr<Expression> value;
  SubExpression(std::shared_ptr<Expression> value) : value{value} {}
  Kind kind() const{ return Kind::SubExpression; }
  void accept(Visitor &v) override;
  std::shared_ptr<Expression> clone() override;
};

struct Decl
{
  enum class Kind { Object, Controller, Experiment };
  virtual Kind kind() const = 0;
};

struct Equation : public ASTNode, 
                  public Clonable<Equation>,
                  public std::enable_shared_from_this<Equation>
{
  std::shared_ptr<Expression> lhs, rhs;

  void accept(Visitor &v) override;
  std::shared_ptr<Equation> clone() override;
};


//Equation Visitor ------------------------------------------------------------
struct Visitor
{
  virtual void visit(std::shared_ptr<Equation>) {}
  virtual void in(std::shared_ptr<Equation>) {}
  virtual void leave(std::shared_ptr<Equation>) {}

  virtual void visit(std::shared_ptr<Add>) {}
  virtual void in(std::shared_ptr<Add>) {}
  virtual void leave(std::shared_ptr<Add>) {}

  virtual void visit(std::shared_ptr<Subtract>) {}
  virtual void in(std::shared_ptr<Subtract>) {}
  virtual void leave(std::shared_ptr<Subtract>) {}

  virtual void visit(std::shared_ptr<Multiply>) {}
  virtual void in(std::shared_ptr<Multiply>) {}
  virtual void leave(std::shared_ptr<Multiply>) {}

  virtual void visit(std::shared_ptr<Divide>) {}
  virtual void in(std::shared_ptr<Divide>) {}
  virtual void leave(std::shared_ptr<Divide>) {}

  virtual void visit(std::shared_ptr<Pow>) {}
  virtual void in(std::shared_ptr<Pow>) {}
  virtual void leave(std::shared_ptr<Pow>) {}

  virtual void visit(std::shared_ptr<Differentiate>) {}
  virtual void in(std::shared_ptr<Differentiate>) {}
  virtual void leave(std::shared_ptr<Differentiate>) {}

  virtual void visit(std::shared_ptr<Symbol>) {}
  virtual void in(std::shared_ptr<Symbol>) {}
  virtual void leave(std::shared_ptr<Symbol>) {}
  
  virtual void visit(std::shared_ptr<Real>) {}
  virtual void in(std::shared_ptr<Real>) {}
  virtual void leave(std::shared_ptr<Real>) {}
  
  virtual void visit(std::shared_ptr<SubExpression>) {}
  virtual void in(std::shared_ptr<SubExpression>) {}
  virtual void leave(std::shared_ptr<SubExpression>) {}
};

//Free functions over equations -----------------------------------------------
std::shared_ptr<Equation> 
setToZero(std::shared_ptr<Equation>);

//Equation Visitors -----------------------------------------------------------
struct EqtnParametizer : public Visitor
{
  std::string symbol_name;
  double value;
  
  void visit(std::shared_ptr<Add>) override;
  void visit(std::shared_ptr<Subtract>) override;
  void visit(std::shared_ptr<Multiply>) override;
  void visit(std::shared_ptr<Divide>) override;
  void visit(std::shared_ptr<Pow>) override;

  template<class BinOp>
  void apply(std::shared_ptr<BinOp> x);
};

void applyParameter(std::shared_ptr<Equation>, std::string symbol_name, 
    double value);

} //::cypress
#include "Equation.hh"
#endif
