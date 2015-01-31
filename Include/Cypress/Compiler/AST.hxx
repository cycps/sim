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
  std::shared_ptr<Term> lhs, rhs; 
};

struct Add : public GroupOp 
{ 
  Kind kind(){ return Kind::Add; } 
};

struct Subtract : public GroupOp 
{
  Kind kind(){ return Kind::Subtract; }
};

struct Factor : public Term {};

struct RingOp : public Term
{
  std::shared_ptr<Factor> lhs, rhs;
};

struct Multiply : public RingOp
{
  Kind kind(){ return Kind::Multiply; }
};

struct Divide : public RingOp
{
  Kind kind(){ return Kind::Divide; }
};

struct Atom : public Factor {};

struct Pow : public Factor
{
  std::shared_ptr<Atom> lhs, rhs;
  Kind kind(){ return Kind::Pow; }
};

struct Differentiate : public Factor
{
  std::shared_ptr<Atom> arg;
  Kind kind(){ return Kind::Differentiate; }
};

struct Symbol : public Atom
{
  std::string name;
  Kind kind(){ return Kind::Symbol; }
};

struct Real : public Atom
{
  double value;
  Kind kind(){ return Kind::Real; }
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
