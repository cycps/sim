#ifndef CYPRESS_SIMEX
#define CYPRESS_SIMEX

#include <ida/ida.h>
#include <ida/ida_dense.h>
#include <nvector/nvector_serial.h>
#include <sundials/sundials_math.h>
#include <sundials/sundials_types.h>

#include <string>
#include <vector>
#include <functional>

namespace cypress
{

struct SimEx
{
  size_t neq;
  double rtol, satol;
  N_Vector y, dy, avtol;
  realtype *yv, *dyv, *avtolv;

  std::string residualClosureSource;

  explicit SimEx(size_t neq, double rtol, double satol);
  explicit SimEx(std::string source);
  void run();
  
  void applySATol();
  void startupReport();

  std::string toString();

  size_t parseNEQ(std::string ln);
  double parseRTOL(std::string ln);
  double parseSATOL(std::string ln);

};

}

#endif
