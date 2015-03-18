#include <Cypress/Sim/ComputeNode.hxx>
#include <Cypress/Sim/Simutron.hxx>

#include <ida/ida.h>
#include <ida/ida_dense.h>
#include <nvector/nvector_serial.h>
#include <sundials/sundials_math.h>
#include <sundials/sundials_types.h>

#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <iomanip>

using std::cout;
using std::endl;
using std::ofstream;
using std::to_string;
using std::runtime_error;
using namespace cypress;
using namespace cypress::sim;

#define IJth(A,i,j) DENSE_ELEM(A,i,j)

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

bool checkInitialConds(double tol);

extern Simutron *rc;


int main()
{
  ofstream ofs{
    rc->experimentInfo() + to_string(rc->id()) + ".log"
  };
  ofs << "Cypress Direct Compute Node ... Engage" << endl;
  ofs << "N=" << rc->N() << endl;
  rc->lg = &ofs;

  rc->nv_y = N_VNew_Serial(rc->N());
  rc->nv_dy = N_VNew_Serial(rc->N());
  rc->nv_r = N_VNew_Serial(rc->N());

  rc->y = NV_DATA_S(rc->nv_y);
  rc->dy = NV_DATA_S(rc->nv_dy);
  rc->r = NV_DATA_S(rc->nv_r);

  rc->init();

  double rtl = 1e-3,
         atl = 1e-6;

  double ts = 0, te = 7;

  void *mem = IDACreate();
  if(mem == nullptr) throw runtime_error{"IDACreate failed"};

  int retval = IDAInit(mem, F, ts, rc->nv_y, rc->nv_dy);
  if(retval != IDA_SUCCESS)
    throw runtime_error("IDAInit failed: " + to_string(retval));

  retval = IDASetUserData(mem, rc);
  if(retval != IDA_SUCCESS)
    throw runtime_error{"IDASetUserData failed: " + to_string(retval)};
  
  retval = IDASStolerances(mem, rtl, atl);
  if(retval != IDA_SUCCESS) 
    throw runtime_error{"IDASVtolerances failed: " + to_string(retval)};
  
  retval = IDADense(mem, rc->N());
  if(retval != IDA_SUCCESS)
    throw runtime_error{"IDADense failed: " + to_string(retval)};
  
  //retval = IDADlsSetDenseJacFn(mem, J);
  //if(retval != IDA_SUCCESS)
  //  throw runtime_error{"IDADlsSetDenseJacFn failed: " + to_string(retval)};
  
  bool init_ok = checkInitialConds(atl);
  if(!init_ok)
  {
    *rc->lg << "Bad Initial Conditions" << endl;
    throw runtime_error("Bad Initial Conditions");
  }
  
  double tret{0};
  for(double tout=0.01; tout<te; tout += 0.01)
  {
    retval = IDASolve(mem, tout, &tret, rc->nv_y, rc->nv_dy, IDA_NORMAL);

    cout << std::setprecision(6) << std::fixed;
    for(size_t i=0; i<rc->N(); ++i) cout << rc->y[i] << ","; 
    for(size_t i=0; i<rc->N()-1; ++i) cout << rc->dy[i] << ","; 
    cout << rc->dy[rc->N()-1];
    cout << endl;

    /*
      << rc->y[0] << "\t"
      << rc->y[1] << "\t"
      << rc->y[2] << "\t"
      << rc->y[3] << "\t"
      << endl;
      */
    
    if(retval != IDA_SUCCESS)
      throw runtime_error{"IDASolve failed: " + to_string(retval)};
  }
  cout << endl;

}

bool checkInitialConds(double tol)
{
  realtype *r = (realtype*)malloc(sizeof(realtype) * rc->L());
  rc->compute(r, 0);
  bool ok{true};

  *rc->lg << "Initial Check" << endl;
  for(size_t i=0; i<rc->L(); ++i)
  {
    *rc->lg << "r[0]: " << r[i] << endl;  
    if(std::abs(r[i]) > tol) ok = false;
  }
  
  return ok;
}

int F(realtype t, N_Vector y, N_Vector dy, N_Vector r, void*)
{
  rc->y = NV_DATA_S(y);
  rc->dy = NV_DATA_S(dy);

  realtype *rv = NV_DATA_S(r);

  rc->compute(rv, t);

  /*
  cout << "~~ "
    << std::setprecision(6) << std::fixed;

  for(int i=0; i<rc->N(); ++i) cout << rc->y[i] << "\t";

  cout << endl;
  */

  return 0;
}

int J(long int N, 
      realtype t, 
      realtype /*cj*/,
      N_Vector nv_y,
      N_Vector nv_dy,
      N_Vector /*r*/,
      DlsMat J,
      void* /*udata*/,
      N_Vector, N_Vector, N_Vector)
{
  realtype *y = NV_DATA_S(nv_y),
           *dy = NV_DATA_S(nv_dy);
  realtype *a = (realtype*)malloc(sizeof(realtype)*N),
           *b = (realtype*)malloc(sizeof(realtype)*N);

  realtype delta = 1e-6;
      
  cout << "~~ J ----------------------------------------------" << endl;
 
  for(long i=0; i<N; ++i)
  {
    rc->compute(a, t);
    for(long j=0; j<N; ++j)
    { 
      y[j] += delta;
      double ody = dy[j];
      dy[j] = delta;
      rc->compute(b, t);
      dy[j] = ody;
      y[j] -= delta;
      realtype del = a[i] - b[i];
      cout << del << " ";

      IJth(J, i, j) = del;
    }
    cout << endl;
  }

  free(a);
  free(b);

  return 0;
}
