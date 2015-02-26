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

namespace cypress
{

struct ResidualClosure
{
  N_Vector nv_y, nv_dy, nv_r;
  realtype *y, *dy, *c, *r;
  MPI_Win ywin, dywin;
  MPI_Comm ycomm, dycomm;
  std::vector<DCoordinate> varmap;
  virtual void compute(realtype *r) = 0;
  virtual void resolve() = 0;
  virtual void init() = 0;
  virtual std::string experimentInfo() = 0;
  virtual size_t id() = 0;
  virtual size_t L() = 0;
  virtual size_t N() = 0;
};

}

#endif
