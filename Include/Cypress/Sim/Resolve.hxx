#ifndef CYPRESS_SIM_RESOLVE_HXX
#define CYPRESS_SIM_RESOLVE_HXX

#include "Cypress/Core/Equation.hxx"
//#include "Cypress/Core/Elements.hxx"
#include <RyMPI/runtime.hxx>
#include <sundials/sundials_types.h>
#include <mpi.h>
#include <string>
#include "Cypress/Core/Var.hxx"

namespace cypress
{
  
  struct Component; using ComponentSP = std::shared_ptr<Component>;

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
  //std::string name;
  VarRefSP var;
  DCoordinate coord;

  RVar(VarRefSP var, DCoordinate coord) : var{var}, coord{coord}
  {}
};

struct REqtn
{
  EquationSP eqtn;
  ComponentSP component;
  DCoordinate coord;

  REqtn(EquationSP eqtn, ComponentSP c, DCoordinate coord) 
    : eqtn{eqtn}, component{c}, coord{coord}
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
