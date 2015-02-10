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

struct ASTNode
{
  virtual void accept(Visitor &) = 0;
};


} //::cypress

#endif
