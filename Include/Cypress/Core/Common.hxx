#ifndef CYPRESS_COMMON
#define CYPRESS_COMMON

#include <memory>
#include <utility>

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
  Lexeme(size_t line, size_t column) : line{line}, column{column} {}
};

struct ASTNode : public Lexeme
{
  virtual void accept(Visitor &) = 0;
  using Lexeme::Lexeme;
};

template<class Type, class Thing>
bool isa(Thing a, Type ty)
{
  return a->kind() == ty;
}



} //::cypress

#endif
