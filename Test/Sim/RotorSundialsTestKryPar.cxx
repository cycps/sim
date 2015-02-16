/******************************************************************************
 * Cypress
 *
 * Rotor Simulation Sundials Example Krylov Parallel
 * -------------------------------------------------
 *  This is the same as ${CYPRESS_ROOT}/Test/Sim/RotorSundialsTest.cxx execpt
 *  for this version uses a newton krylov solver that exploits parallel arch
 *  through MPI.
 *
 */

#include <ida/ida.h>
//#include <ida/ida_spils.h>
#include <ida/ida_spgmr.h>
#include <ida/ida_bbdpre.h>
#include <nvector/nvector_parallel.h>
#include <sundials/sundials_math.h>
#include <sundials/sundials_types.h>

#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <string>
#include <cmath>
#include <vector>
#include <functional>

using std::cout;
using std::endl;
using std::runtime_error;
using std::to_string;
using std::vector;
using std::function;

static constexpr size_t NEQ{4};


struct RotorParams
{
  realtype H, T;
};

struct MPIConfig { int N{-1}, r{-1}, L{-1}; };

struct RotorEqtns
{
  realtype *y, *dy, t;
  RotorParams *P;
  inline realtype ω(){ return y[0]; } 
  inline realtype dω(){ return dy[0]; }
  inline realtype dθ(){ return dy[1]; }
  inline realtype τ(){ return y[2]; }
  inline realtype a(){ return y[3]; }
  inline realtype H() { return P->H; }
  inline realtype T() { return P->T; }

  #define EQ(__EQTN__) [this](){ return __EQTN__; }
  vector<function<realtype()>> eqtns
  {
    EQ(dθ() - ω()),
    EQ(τ() - H()*pow(ω(),2) - a()),
    EQ(dω() - a()),
    EQ(T() + T()*sin(t - M_PI/2) - τ())
  };
};

struct UData
{
  RotorParams rotorParams;
  RotorEqtns rotorEqtns;
  MPIConfig mpic;

  UData(){ rotorEqtns.P = &rotorParams; }
};

int F(realtype t, N_Vector y, N_Vector dy, N_Vector r, void *udata);
int FL(long int L, realtype t, N_Vector y, N_Vector dy, N_Vector r, void *udata);

void initData(realtype *y, realtype *dy, MPIConfig mpic)
{
  //size_t start = mpic.r * mpic.L;
  //size_t end = start + mpic.L;

  for(int i=0; i<mpic.L; ++i)
  {
    y[i] = 0.0;
    dy[i] = 0.0;
  }
}

int main(int argc, char **argv)
{
  cout << "Sundials Rotor Test Parallel" << endl;

  MPI_Init(&argc, &argv);
  MPI_Comm comm{MPI_COMM_WORLD};
  MPIConfig mpic;
  MPI_Comm_size(comm, &mpic.N);
  MPI_Comm_rank(comm, &mpic.r);
  mpic.L = NEQ/mpic.N;

  UData udata;
  udata.mpic = mpic;

  size_t L = mpic.L;

  cout << "L:" << L << endl;
   
  N_Vector 
    y{N_VNew_Parallel(comm, L, NEQ)},
    dy{N_VNew_Parallel(comm, L, NEQ)};

  realtype *yval{NV_DATA_P(y)},
           *dyval{NV_DATA_P(dy)};
  
  initData(yval, dyval, mpic);
  /*
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
  */
  //relative tolerance
  double rtl = 0;
  double atl = 1.0e-3;
  
  //Simulation starting and ending times
  double ts = 0, te = 7;

  //Init IDA
  void *mem = IDACreate();
  if(mem == nullptr) throw runtime_error{"IDACreate failed"};
  
  int retval = IDAInit(mem, F, ts, y, dy);
  if(retval != IDA_SUCCESS) 
    throw runtime_error{"IDAInit failed: " + to_string(retval)};

  udata.rotorParams.H = 2.5;
  udata.rotorParams.T = 5;
  retval = IDASetUserData(mem, &udata);
  if(retval != IDA_SUCCESS)
    throw runtime_error{"IDASetUserData failed: " + to_string(retval)};

  retval = IDASStolerances(mem, rtl, atl);
  if(retval != IDA_SUCCESS) 
    throw runtime_error{"IDASVtolerances failed: " + to_string(retval)};

  retval = IDASpgmr(mem, 0);
  if(retval != IDA_SUCCESS)
    throw runtime_error{"IDASpgmr failed: " + to_string(retval)};

  retval = IDABBDPrecInit(mem, L, 2, 2, 1, 1, 0.0, FL, NULL);
  if(retval != IDA_SUCCESS)
    throw runtime_error{"IDABBDPrecInit failed: " + to_string(retval)};

  double tret{0};
  for(double tout=0.01; tout<te; tout += 0.01)
  {
    retval = IDASolve(mem, tout, &tret, y, dy, IDA_NORMAL);
    if(retval != IDA_SUCCESS)
      throw runtime_error{"IDASolve failed " + to_string(retval)};
    
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
  auto data = reinterpret_cast<UData*>(udata);
  FL(data->mpic.L, t, y, dy, r, udata);

  MPI_Barrier(MPI_COMM_WORLD);
  return 0;
}

int FL(long int L, realtype t, N_Vector y, N_Vector dy, N_Vector r, void *udata)
{
  realtype *rv = NV_DATA_P(r);

  UData *data = reinterpret_cast<UData*>(udata);

  data->rotorEqtns.y = NV_DATA_P(y);
  data->rotorEqtns.dy = NV_DATA_P(dy);
  data->rotorEqtns.t = t;

  size_t start = data->mpic.r * L;
  size_t end = start + L;

  for(size_t il=0, ig=start; il<L && ig<end; ++ig, ++il)
    rv[il] = data->rotorEqtns.eqtns[ig]();

  
  return 0;
}
