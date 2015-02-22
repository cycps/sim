#ifndef CYPRESS_SIM_RESOLVE_HXX
#define CYPRESS_SIM_RESOLVE_HXX

#include "Cypress/Core/Equation.hxx"
#include "Cypress/Sim/Var.hxx"
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

static inline
realtype yresolve(DCoordinate)
{
  return 0;
}

static inline
realtype dyresolve(DCoordinate)
{
  return 0;
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
