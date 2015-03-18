#ifndef CYPRESS_SIM_RESIDUALCLOSURE
#define CYPRESS_SIM_RESIDUALCLOSURE

#include <Cypress/Sim/Resolve.hxx>

#include <ida/ida.h>
#include <ida/ida_dense.h>
#include <nvector/nvector_serial.h>
#include <sundials/sundials_math.h>
#include <sundials/sundials_types.h>

#include <vector>
#include <functional>
#include <string>
#include <iostream>

namespace cypress { namespace sim {

struct Simutron
{
  N_Vector nv_y, nv_dy, nv_r;
  realtype *y, *dy, *c, *r;
  realtype *ry, *rdy;
  MPI_Win ywin, dywin;
  MPI_Comm ycomm, dycomm;
  std::ostream *lg;

  //maps hash(who+what) to a local input index
  std::unordered_map<unsigned long, size_t> cmap;

  std::vector<DCoordinate> varmap;
  void clisten();
  virtual void compute(realtype *r, realtype t) = 0;
  virtual void resolve() = 0;
  virtual void init() = 0;
  virtual std::string experimentInfo() = 0;
  virtual size_t id() = 0;
  virtual size_t L() = 0;
  virtual size_t N() = 0;
};

}} //::cypress::sim

#endif
