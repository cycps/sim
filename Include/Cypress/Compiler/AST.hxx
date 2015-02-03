#ifndef CYPRESS_COMPILE_AST_HXX
#define CYPRESS_COMPILE_AST_HXX

#include <vector>
#include <memory>
#include <string>
#include <iostream>
#include <regex>

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
  virtual Kind kind() const = 0;
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
  Kind kind() const{ return Kind::Add; } 
  using GroupOp::GroupOp;
};

struct Subtract : public GroupOp 
{
  Kind kind() const{ return Kind::Subtract; }
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
  Kind kind() const{ return Kind::Multiply; }
  using RingOp::RingOp;
};

struct Divide : public RingOp
{
  Kind kind() const{ return Kind::Divide; }
  using RingOp::RingOp;
};

struct Atom : public Factor {};

struct Pow : public Factor
{
  std::shared_ptr<Atom> lhs, rhs;
  Kind kind() const{ return Kind::Pow; }
  Pow(std::shared_ptr<Atom> lhs, std::shared_ptr<Atom> rhs)
    : lhs{lhs}, rhs{rhs} {}
};

struct Symbol : public Atom
{
  std::string value;
  Kind kind() const{ return Kind::Symbol; }
  Symbol(std::string value) : value{value} {}
};

struct Differentiate : public Factor
{
  std::shared_ptr<Symbol> arg;
  Kind kind() const{ return Kind::Differentiate; }
  Differentiate(std::shared_ptr<Symbol> arg) : arg{arg} {}
};

struct Real : public Atom
{
  double value;
  Kind kind() const{ return Kind::Real; }
  Real(double value) : value{value} {}
};

struct SubExpression : public Atom
{
  std::shared_ptr<Expression> value;
  Kind kind() const{ return Kind::SubExpression; }
};

struct Decl
{
  enum class Kind { Object, Controller, Experiment };
  virtual Kind kind() const = 0;
};

struct Equation
{
  std::shared_ptr<Expression> lhs, rhs;
};

struct Object : public Decl
{
  std::shared_ptr<Symbol> name;
  std::vector<std::shared_ptr<Symbol>> params;
  std::vector<std::shared_ptr<Equation>> eqtns; 
  Kind kind() const override { return Kind::Object; }
  Object(std::shared_ptr<Symbol> name) : name{name} {}
};

struct Controller : public Decl
{
  std::vector<std::shared_ptr<Equation>> eqtns;
  Kind kind() const override { return Kind::Controller; }
};

struct Experiment : public Decl
{
  Kind kind() const override { return Kind::Experiment; }
};

struct Decls
{
  std::vector<std::shared_ptr<Object>> objects;
  std::vector<std::shared_ptr<Controller>> controllers;
  std::vector<std::shared_ptr<Experiment>> experiments;
};

std::ostream& operator << (std::ostream &o, const Decls &d);
std::ostream& operator << (std::ostream &o, const Object &obj);
std::ostream& operator << (std::ostream &o, const Controller &controller);
void showEqtn(std::ostream &, const Equation &);
void showExpr(size_t indent, std::ostream &o, const Expression &expr);
  
static std::regex objrx{"Object\\s*([a-zA-Z_]+)(\\(.*\\))"};
static std::regex paramsrx{"\\(([a-zA-Zα-ωΑ-Ω_][a-zA-Zα-ωΑ-Ω_0-9_]*)"
                           "(?:,([a-zA-Zα-ωΑ-Ω_][a-zA-Zα-ωΑ-Ω_0-9_]*))*\\)"};

}}

#endif
