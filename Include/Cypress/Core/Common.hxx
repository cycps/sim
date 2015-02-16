#ifndef CYPRESS_COMMON
#define CYPRESS_COMMON

#include <memory>

//Forward declarations --------------------------------------------------------
namespace cypress
{
  struct Visitor;
}

namespace cypress
{

template<class T>
struct Clonable
{
  virtual std::shared_ptr<T> clone() = 0;
};

struct Lexeme
{
  size_t line, column;
  explicit Lexeme(size_t line, size_t column=0) : line{line}, column{column} {}
};

struct ASTNode : public Lexeme
{
  virtual void accept(Visitor &) = 0;
  ASTNode(size_t line) : Lexeme{line} {}
};

template<class Type, class Thing>
bool isa(Thing a, Type ty)
{
  return a->kind() == ty;
}



} //::cypress

#endif
