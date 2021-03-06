#include <Cypress/Sim/ComputeNode.hxx>
#include <Cypress/Sim/Simutron.hxx>
#include <RyMPI/Runtime.hxx>

#include <ida/ida.h>
#include <ida/ida_spgmr.h>
#include <ida/ida_bbdpre.h>
#include <nvector/nvector_parallel.h>
#include <sundials/sundials_math.h>
#include <sundials/sundials_types.h>

#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>

using std::cout;
using std::endl;
using std::ofstream;
using std::to_string;
using std::runtime_error;
using namespace cypress;
using namespace cypress::sim;

extern Simutron *rc;

int F(realtype t, N_Vector y, N_Vector dy, N_Vector r, void *udata);
int FL(long int L, realtype t, N_Vector y, N_Vector dy, N_Vector r, void *udata);
bool checkInitialConds();

int main(int argc, char **argv)
{
  ofstream ofs{
    rc->experimentInfo() + to_string(rc->id()) + ".log"
  };
  ofs << "Cypress Compute Node ... Engage" << endl;
  ofs << "N=" << rc->N() << ", " << "L=" << rc->L() << endl;

  RyMPI::Runtime rt(&argc, &argv);
  RyMPI::CommInfo ci = rt.commInfo();
  MPI_Comm_dup(ci.comm, &rc->ycomm);
  MPI_Comm_dup(ci.comm, &rc->dycomm);

  //init residual closure nvectors
  rc->nv_y = N_VNew_Parallel(rc->ycomm, rc->L(), rc->N());
  rc->nv_dy = N_VNew_Parallel(rc->dycomm, rc->L(), rc->N());
  rc->nv_r = N_VNew_Parallel(MPI_COMM_WORLD, rc->L(), rc->N());

  rc->y = NV_DATA_P(rc->nv_y);
  rc->dy = NV_DATA_P(rc->nv_dy);
  rc->r = NV_DATA_P(rc->nv_r);
  
  rc->init();

  //TODO: Kill hardcode
  //tolerances
  double rtl = 1.0e-3,
         atl = 1.0e-3;

  //TODO: Kill hardcode
  //simulation start and end times
  double ts = 0, te = 7;

  //Init IDA
  void *mem = IDACreate();
  if(mem == nullptr)
  {
    ofs << "IDA Create Failed" << endl;
    exit(1);
  }

  int retval = IDAInit(mem, F, ts, rc->nv_y, rc->nv_dy);
  if(retval != IDA_SUCCESS)
  {
    ofs << "IDAInit failed: " << retval << endl;
    exit(1);
  }

  retval = IDASetUserData(mem, rc);
  if(retval != IDA_SUCCESS)
  {
    ofs << "IDASetUserData failed: " << retval << endl;
    exit(1);
  }

  retval = IDASStolerances(mem, rtl, atl);
  if(retval != IDA_SUCCESS)
  {
    ofs << "IDASStolerances failed: " << retval << endl;
    exit(1);
  }

  retval = IDASpgmr(mem, 0);
  if(retval != IDA_SUCCESS)
  {
    ofs << "IDASpgmr failed: " << retval << endl;
    exit(1);
  }

  retval = IDABBDPrecInit(mem, rc->L(), 2, 2, 1, 1, 0.0, FL, NULL);
  if(retval != IDA_SUCCESS)
  {
    ofs << "IDABBDPrecInit failed: " << retval << endl;
    exit(1);
  }

  bool init_ok = checkInitialConds();
  if(!init_ok)
  {
    rc->c_lg << "Bad Initial Conditions" << endl;
    throw runtime_error("Bad Initial Conditions");
  }

  //TODO: no hardcode step
  for(double tout=0.01, tret=0; tout<te; tout += 0.01)
  {
    retval = IDASolve(mem, tout, &tret, rc->nv_y, rc->nv_dy, IDA_NORMAL);
    if(retval != IDA_SUCCESS)
    {
      ofs << "IDASolve failed: " << retval << endl;
      exit(1);
    }

    for(size_t i=0; i<rc->L(); ++i)
      ofs << rc->y[i] << ",";
    ofs << endl;
    
  }

  return 0;
}

bool checkInitialConds()
{
  realtype *r = (realtype*)malloc(sizeof(realtype) * rc->L());
  rc->compute(r, 0);
  bool ok{true};

  rc->c_lg << "Initial Check" << endl;
  for(size_t i=0; i<rc->L(); ++i)
  {
    rc->c_lg << "r[0]: " << r[i] << endl;  
    if(std::abs(r[i]) > 1e-6) ok = false;
  }
  
  return ok;
}

int F(realtype t, N_Vector y, N_Vector dy, N_Vector r, void *udata)
{
  Simutron *rc = static_cast<Simutron*>(udata);
  rc->resolve();
  FL(rc->L(), t, y, dy, r, udata);

  return 0;  
}

int FL(long int /*L*/, realtype t, N_Vector y, N_Vector dy, 
    N_Vector r, void *udata)
{
  Simutron *rc = static_cast<Simutron*>(udata);

  realtype *rv = NV_DATA_P(r);
  rc->y = NV_DATA_P(y);
  rc->dy = NV_DATA_P(dy);
  
  rc->compute(rv, t);

  return 0;  
}
