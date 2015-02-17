/******************************************************************************
 * Cypress
 *
 * Rotor Simulation Sundials Example
 * -----------------------------------
 * This is an example of how to use sundials to simulate a spinning rotor the
 * equations are:
 *
 *  ω = θ'
 *  a = τ - H*ω^2
 *  a = ω'
 *  τ = T + T*sin(t - π/2)
 *
 * The symbols represent the following:
 *
 *  Dependent Variables:
 *    ω : angular velocity of the rotor
 *    θ : angular position of the rotor
 *    τ : torque being applied to the rotor
 *    a : accelleration of the rotor
 *
 *  Constant Parameters:
 *    T : constant torque coefficient
 *    H : Inertial constant of the rotor
 *
 *  Independent Variables:
 *    t : time
 *
 * The dependent variable - vector space mapping is:
 *
 *       0  1  2  3
 *  y = {ω, θ, τ, a}
 *
 * T and H are a constant parameters and t is independent so they are not 
 * included in the space
 *
 */

#include <ida/ida.h>
#include <ida/ida_dense.h>
#include <nvector/nvector_serial.h>
#include <sundials/sundials_math.h>
#include <sundials/sundials_types.h>

#include "gtest/gtest.h"
#include <iostream>
#include <stdexcept>
#include <string>
#include <cmath>

#define IJth(A,i,j) DENSE_ELEM(A,i,j)

using std::cout;
using std::endl;
using std::runtime_error;
using std::to_string;

static constexpr size_t NEQ{4};

int F(realtype t, N_Vector y, N_Vector dy, N_Vector r, void *udata);

int J(long int Neq, 
      realtype t, 
      realtype cj,
      N_Vector y,
      N_Vector dy,
      N_Vector r,
      DlsMat J,
      void *udata,
      N_Vector tmp1, N_Vector tmp2, N_Vector tmp3);

struct RotorParams
{
  realtype H, T;
};

TEST(Sim, SundialsRotorSerial)
{
  cout << "Sundials Rotor Test" << endl;

  N_Vector 
    y{N_VNew_Serial(NEQ)}, 
    dy{N_VNew_Serial(NEQ)}, 
    avtol{N_VNew_Serial(NEQ)};

  realtype *yval{NV_DATA_S(y)},
           *dyval{NV_DATA_S(dy)},
           *atval{NV_DATA_S(avtol)};

  yval[0] = 0.0; //initial velocity is zero
  yval[1] = 0.0; //initial position is zero
  yval[2] = 0.0; //initial torque is zero
  yval[3] = 0.0; //initial accelleration is zero

  dyval[0] = 0.0; //initial velocity rate of change is zero
  dyval[1] = 0.0; //initial position rate of change is zero
  dyval[2] = 0.0; //initial torque rate of change is zero
  dyval[3] = 0.0; //initial acceleration rate of change is zero

  //uniform absolute tolerance values
  atval[0] = 
  atval[1] = 
  atval[2] = 
  atval[3] = 1.0e-6;
  //relative tolerance
  double rtol = 1.0e-4;

  //Simulation starting and ending times
  double ts = 0, te = 7;

  //Init IDA
  void *mem = IDACreate();
  if(mem == nullptr) throw runtime_error{"IDACreate failed"};

  int retval = IDAInit(mem, F, ts, y, dy);
  if(retval != IDA_SUCCESS) 
    throw runtime_error{"IDAInit failed: " + to_string(retval)};

  RotorParams rt;
  rt.H = 2.5;
  rt.T = 5;
  retval = IDASetUserData(mem, &rt);
  if(retval != IDA_SUCCESS)
    throw runtime_error{"IDASetUserData failed: " + to_string(retval)};

  retval = IDASVtolerances(mem, rtol, avtol);
  if(retval != IDA_SUCCESS) 
    throw runtime_error{"IDASVtolerances failed: " + to_string(retval)};

  N_VDestroy_Serial(avtol);

  retval = IDADense(mem, NEQ);
  if(retval != IDA_SUCCESS)
    throw runtime_error{"IDADense failed: " + to_string(retval)};

  retval = IDADlsSetDenseJacFn(mem, J);
  if(retval != IDA_SUCCESS)
    throw runtime_error{"IDADlsSetDenseJacFn failed: " + to_string(retval)};

  double tret{0};
  for(double tout=0.01; tout<te; tout += 0.01)
  {
    retval = IDASolve(mem, tout, &tret, y, dy, IDA_NORMAL);
    if(retval != IDA_SUCCESS)
      throw runtime_error{"IDASolve failed: " + to_string(retval)};

    cout 
      << std::setprecision(6) << std::fixed
      << yval[0] << "\t"
      << yval[1] << "\t"
      << yval[2] << "\t"
      << yval[3] << "\t"
      << endl;
  }
  cout << endl;

}

int F(realtype t, N_Vector y, N_Vector dy, N_Vector r, void *udata)
{
  realtype *yval = NV_DATA_S(y),
           *dyval = NV_DATA_S(dy),
           *rv = NV_DATA_S(r);

  RotorParams *params = reinterpret_cast<RotorParams*>(udata);
  realtype H = params->H,
           T = params->T;

  double ω = yval[0],  
         dω = dyval[0], 
         dθ = dyval[1],
         τ = yval[2],  
         a = yval[3];

  rv[0] = dθ - ω;
  rv[1] = τ - H*pow(ω,2) - a;
  rv[2] = dω - a;
  rv[3] = T + T*sin(t - M_PI/2) - τ;

  return 0;
}

int J(long int, 
      realtype, 
      realtype cj,
      N_Vector y,
      N_Vector,
      N_Vector,
      DlsMat J,
      void *udata,
      N_Vector, N_Vector, N_Vector)
{
  realtype *yv{NV_DATA_S(y)};

  double ω = yv[0];
  
  RotorParams *params = reinterpret_cast<RotorParams*>(udata);
  realtype H = params->H;

  /*dF0/dω*/ IJth(J,0,0) = -1;
  /*dF0/dθ*/ IJth(J,0,1) = cj;
  /*dF0/dτ*/ IJth(J,0,2) = 0;
  /*dF0/da*/ IJth(J,0,3) = 0;

  /*dF1/dω*/ IJth(J,1,0) = 2*H*ω;
  /*dF1/dθ*/ IJth(J,1,1) = 0;
  /*dF1/dτ*/ IJth(J,1,2) = 1;
  /*dF1/da*/ IJth(J,1,3) = -1;

  /*dF2/dω*/ IJth(J,2,0) = cj;
  /*dF2/dθ*/ IJth(J,2,1) = 0;
  /*dF2/dτ*/ IJth(J,2,2) = 0;
  /*dF2/da*/ IJth(J,2,3) = -1;

  /*dF3/dω*/ IJth(J,3,0) = 0;
  /*dF3/dθ*/ IJth(J,3,1) = 0;
  /*dF3/dτ*/ IJth(J,3,2) = -1;
  /*dF3/da*/ IJth(J,3,3) = 0;

  return 0;
}
