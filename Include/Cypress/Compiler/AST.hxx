#ifndef CYPRESS_COMPILE_AST_HXX
#define CYPRESS_COMPILE_AST_HXX

#include <vector>
#include <memory>
#include <string>

namespace cypress { namespace compile {

struct Expression
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
  virtual Kind kind() = 0;
};

struct Term : public Expression {};

struct GroupOp : public Expression 
{ 
  std::shared_ptr<Term> lhs;
  std::shared_ptr<Expression> rhs;
  GroupOp(std::shared_ptr<Term> lhs, std::shared_ptr<Expression> rhs) 
    : lhs{lhs}, rhs{rhs} {}
};

struct Add : public GroupOp 
{ 
  Kind kind(){ return Kind::Add; } 
  using GroupOp::GroupOp;
};

struct Subtract : public GroupOp 
{
  Kind kind(){ return Kind::Subtract; }
  using GroupOp::GroupOp;
};

struct Factor : public Term {};

struct RingOp : public Term
{
  std::shared_ptr<Factor> lhs;
  std::shared_ptr<Term> rhs;
  RingOp(std::shared_ptr<Factor> lhs, std::shared_ptr<Term> rhs)
    : lhs{lhs}, rhs{rhs} {}
};

struct Multiply : public RingOp
{
  Kind kind(){ return Kind::Multiply; }
  using RingOp::RingOp;
};

struct Divide : public RingOp
{
  Kind kind(){ return Kind::Divide; }
  using RingOp::RingOp;
};

struct Atom : public Factor {};

struct Pow : public Factor
{
  std::shared_ptr<Atom> lhs, rhs;
  Kind kind(){ return Kind::Pow; }
  Pow(std::shared_ptr<Atom> lhs, std::shared_ptr<Atom> rhs)
    : lhs{lhs}, rhs{rhs} {}
};

struct Symbol : public Atom
{
  std::string name;
  Kind kind(){ return Kind::Symbol; }
  Symbol(std::string name) : name{name} {}
};

struct Differentiate : public Factor
{
  std::shared_ptr<Symbol> arg;
  Kind kind(){ return Kind::Differentiate; }
  Differentiate(std::shared_ptr<Symbol> arg) : arg{arg} {}
};

struct Real : public Atom
{
  double value;
  Kind kind(){ return Kind::Real; }
  Real(double value) : value{value} {}
};

struct SubExpression : public Atom
{
  std::shared_ptr<Expression> value;
  Kind kind(){ return Kind::SubExpression; }
};

struct Equation
{
  std::shared_ptr<Expression> lhs, rhs;
};

struct Object
{
  std::vector<std::shared_ptr<Equation>> eqtns; 
};


}}

#endif
