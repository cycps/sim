#ifndef CYPRESS_COMMON
#define CYPRESS_COMMON

#include <memory>
#include <utility>
#include <chrono>
#include <string>

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

inline
std::string log(std::string msg)
{
  std::time_t t = std::time(nullptr);
  char ts[128];
  std::strftime(ts, sizeof(ts), "%F %T", std::localtime(&t));
  
  return std::string("[") + std::string(ts) + "] " + msg;
}

inline
std::string ts()
{
  std::time_t t = std::time(nullptr);
  char ts[128];
  std::strftime(ts, sizeof(ts), "%F %T", std::localtime(&t));
  
  return std::string(ts);
}


} //::cypress

#endif
