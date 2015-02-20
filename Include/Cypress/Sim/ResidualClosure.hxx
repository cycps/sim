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
  N_Vector nv_y, nv_dy;
  realtype *y, *dy;
  std::vector<DCoordinate> varmap;
  virtual void compute(realtype *r) = 0;
  virtual std::string experimentInfo() = 0;
};

}

#endif
