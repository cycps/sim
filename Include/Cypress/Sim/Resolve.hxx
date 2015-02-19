#ifndef CYPRESS_SIM_RESOLVE_HXX
#define CYPRESS_SIM_RESOLVE_HXX

#include <sundials/sundials_types.h>
#include <mpi.h>

namespace cypress
{

struct VarCoordinate
{
  size_t px, //process index
         lx; //local var index
};

static inline
realtype yresolve(VarCoordinate)
{
  return 0;
}

static inline
realtype dyresolve(VarCoordinate)
{
  return 0;
}

static inline
realtype __resolve(VarCoordinate)
{
  return 0;
}

}

#endif
