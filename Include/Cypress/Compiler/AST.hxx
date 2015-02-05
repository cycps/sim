#ifndef CYPRESS_COMPILE_AST_HXX
#define CYPRESS_COMPILE_AST_HXX

#include <vector>
#include <memory>
#include <string>
#include <iostream>
#include <regex>
#include <unordered_map>
#include <set>

namespace cypress { namespace compile {

struct Visitor;

struct ASTNode
{
  virtual void accept(Visitor &) = 0;
};

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

struct Visitor
{
  virtual void visit(Equation*) {}
  virtual void leave(Equation*) {}

  virtual void visit(Add*) {}
  virtual void leave(Add*) {}

  virtual void visit(Subtract*) {}
  virtual void leave(Subtract*) {}

  virtual void visit(Multiply*) {}
  virtual void leave(Multiply*) {}

  virtual void visit(Divide*) {}
  virtual void leave(Divide*) {}

  virtual void visit(Pow*) {}
  virtual void leave(Pow*) {}

  virtual void visit(Differentiate*) {}
  virtual void leave(Differentiate*) {}

  virtual void visit(Symbol*) {}
  virtual void leave(Symbol*) {}
  
  virtual void visit(Real*) {}
  virtual void leave(Real*) {}
  
  virtual void visit(SubExpression*) {}
  virtual void leave(SubExpression*) {}
};

struct Expression : public ASTNode
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
  void accept(Visitor &v) override
  {
    v.visit(this);
    lhs->accept(v);
    rhs->accept(v);
    v.leave(this);
  }
};

struct Subtract : public GroupOp 
{
  Kind kind() const{ return Kind::Subtract; }
  using GroupOp::GroupOp;
  void accept(Visitor &v) override
  {
    v.visit(this);
    lhs->accept(v);
    rhs->accept(v);
    v.leave(this);
  }
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
  void accept(Visitor &v) override
  {
    v.visit(this);
    lhs->accept(v);
    rhs->accept(v);
    v.leave(this);
  }
};

struct Divide : public RingOp
{
  Kind kind() const{ return Kind::Divide; }
  using RingOp::RingOp;
  void accept(Visitor &v) override
  {
    v.visit(this);
    lhs->accept(v);
    rhs->accept(v);
    v.leave(this);
  }
};

struct Atom : public Factor {};

struct Pow : public Factor
{
  std::shared_ptr<Atom> lhs, rhs;
  Kind kind() const{ return Kind::Pow; }
  Pow(std::shared_ptr<Atom> lhs, std::shared_ptr<Atom> rhs)
    : lhs{lhs}, rhs{rhs} {}
  
  void accept(Visitor &v) override
  {
    v.visit(this);
    lhs->accept(v);
    rhs->accept(v);
    v.leave(this);
  }
};

struct Symbol : public Atom
{
  std::string value;
  Kind kind() const{ return Kind::Symbol; }
  Symbol(std::string value) : value{value} {}
  void accept(Visitor &v) override
  {
    v.visit(this);
    v.leave(this);
  }
};

struct SymbolCompare
{
  bool operator()(const std::shared_ptr<Symbol> a, 
      const std::shared_ptr<Symbol> b)
  {
    return a->value == b->value;
  }
};

struct Differentiate : public Factor
{
  std::shared_ptr<Symbol> arg;
  Kind kind() const{ return Kind::Differentiate; }
  Differentiate(std::shared_ptr<Symbol> arg) : arg{arg} {}
  void accept(Visitor &v) override
  {
    v.visit(this);
    arg->accept(v);
    v.leave(this);
  }
};

struct Real : public Atom
{
  double value;
  Kind kind() const{ return Kind::Real; }
  Real(double value) : value{value} {}
  void accept(Visitor &v) override
  {
    v.visit(this);
    v.leave(this);
  }
};

struct SubExpression : public Atom
{
  std::shared_ptr<Expression> value;
  Kind kind() const{ return Kind::SubExpression; }
  void accept(Visitor &v) override
  {
    v.visit(this);
    //TODO
    v.leave(this);
  }
};

struct Decl
{
  enum class Kind { Object, Controller, Experiment };
  virtual Kind kind() const = 0;
};

struct Equation : public ASTNode
{
  std::shared_ptr<Expression> lhs, rhs;

  void accept(Visitor &v) 
  {
    v.visit(this);
    lhs->accept(v);
    rhs->accept(v);
    v.leave(this);
  }
};

struct Object : public Decl
{
  std::shared_ptr<Symbol> name;
  std::vector<std::shared_ptr<Symbol>> params;
  std::vector<std::shared_ptr<Equation>> eqtns; 
  Kind kind() const override { return Kind::Object; }
  Object(std::shared_ptr<Symbol> name) : name{name} {}
  std::set<std::shared_ptr<Symbol>, SymbolCompare> vars();
};

struct Controller : public Decl
{
  std::shared_ptr<Symbol> name;
  std::vector<std::shared_ptr<Symbol>> params;
  std::vector<std::shared_ptr<Equation>> eqtns;
  Kind kind() const override { return Kind::Controller; }
  Controller(std::shared_ptr<Symbol> name) : name{name} {}
};

struct Component
{
  std::shared_ptr<Symbol> kind, name;
  std::unordered_map<std::shared_ptr<Symbol>, std::shared_ptr<Real>> params;
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

std::ostream& operator << (std::ostream &o, const Decls &d);
std::ostream& operator << (std::ostream &o, const Object &obj);
std::ostream& operator << (std::ostream &o, const Controller &controller);
std::ostream& operator << (std::ostream &o, const Experiment &expr);
std::ostream& operator << (std::ostream &o, const Component &cp);
std::ostream& operator << (std::ostream &o, const Link &lnk);
std::ostream& operator << (std::ostream &o, const Linkable &lkb);
void showEqtn(std::ostream &, const Equation &);
void showExpr(size_t indent, std::ostream &o, const Expression &expr);
  
static inline std::regex& objrx()
{
  static std::regex *rx = 
    new std::regex{"Object\\s+([a-zA-Z_][a-zA-Z0-9_]*)(\\(.*\\))"};
  return *rx;
}

static inline std::regex& contrx()
{
  static std::regex *rx = 
    new std::regex{"Controller\\s+([a-zA-Z_][a-zA-Z0-9_]*)(\\(.*\\))"};
  return *rx;
}

static inline std::regex& exprx()
{
  static std::regex *rx = 
    new std::regex{"Experiment\\s+([a-zA-Z_][a-zA-Z0-9_]*)"};
  return *rx;
}

static inline std::regex& comprx()
{
  static std::regex *rx = 
    new std::regex{"\\s\\s+([a-zA-Z_][a-zA-Z0-9_]*)\\s+([a-zA-Z_][a-zA-Z0-9_]*)(\\(.*\\))*"};
  return *rx;
}
static inline std::regex& lnkrx()
{
  static std::regex *rx = 
    new std::regex{"\\s\\s+[a-zα-ωΑ-ΩA-Z_][a-zα-ωΑ-ΩA-Z0-9_\\.]*\\s+>\\s+.*"};
  return *rx;
}

static inline std::regex& thingrx()
{
  static std::regex *rx = 
    new std::regex{"([a-zα-ωΑ-ΩA-Z_][a-zα-ωΑ-ΩA-Z0-9_]*)"};
  return *rx;
}

static inline std::regex& subthingrx()
{
  static std::regex *rx = 
    new std::regex{"([a-zα-ωΑ-ΩA-Z_][a-zα-ωΑ-ΩA-Z0-9_]*)\\.([a-zα-ωΑ-ΩA-Z_][a-zα-ωΑ-ΩA-Z0-9_]*)"};
  return *rx;
}

static inline std::regex& atodrx()
{
  static std::regex *rx = 
    new std::regex{"\\|([0-9]+\\.[0-9]+)\\|"};
  return *rx;
}


}}

#endif
