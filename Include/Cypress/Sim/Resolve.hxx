#ifndef CYPRESS_SIM_RESOLVE_HXX
#define CYPRESS_SIM_RESOLVE_HXX

#include "Cypress/Core/Equation.hxx"
#include <RyMPI/runtime.hxx>
#include <sundials/sundials_types.h>
#include <mpi.h>
#include <string>

namespace cypress
{

struct DCoordinate
{
  size_t 
    px, //process index
    gx, //global ver index
    lx; //local var index

  DCoordinate(size_t px, size_t gx, size_t lx) : px{px}, gx{gx}, lx{lx}
  {}
};

//TODO: RVar not a good name
struct RVar
{
  std::string name;
  DCoordinate coord;

  RVar(std::string name, DCoordinate coord) : name{name}, coord{coord}
  {}
};

struct REqtn
{
  EquationSP eqtn;
  DCoordinate coord;

  REqtn(EquationSP eqtn, DCoordinate coord) : eqtn{eqtn}, coord{coord}
  {}
};

//TODO: yresolve & dyresolve are now the same, collapse them at some point
static inline
void yresolve(DCoordinate dc, realtype *r, MPI_Win w)
{
  //MPI_Win_fence(0, w);
  RyMPI::scalarGet(r, dc.px, dc.lx, w);
  //MPI_Win_fence(0, w);
}

static inline
void dyresolve(DCoordinate dc, realtype *r, MPI_Win w)
{
  //MPI_Win_fence(0, w);
  RyMPI::scalarGet(r, dc.px, dc.lx, w);
  //MPI_Win_fence(0, w);
}

static inline
realtype cxresolve(size_t)
{
  return 0;
}

static inline
realtype __resolve(DCoordinate)
{
  return 0;
}

}

#endif
