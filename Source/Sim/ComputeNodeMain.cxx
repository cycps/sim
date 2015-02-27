#include <Cypress/Sim/ComputeNode.hxx>
#include <Cypress/Sim/ResidualClosure.hxx>
#include <RyMPI/runtime.hxx>

#include <ida/ida.h>
#include <ida/ida_spgmr.h>
#include <ida/ida_bbdpre.h>
#include <nvector/nvector_parallel.h>
#include <sundials/sundials_math.h>
#include <sundials/sundials_types.h>

#include <iostream>
#include <fstream>
#include <string>

using std::cout;
using std::endl;
using std::ofstream;
using std::to_string;
using namespace cypress;

extern ResidualClosure *rc;

int F(realtype t, N_Vector y, N_Vector dy, N_Vector r, void *udata);
int FL(long int L, realtype t, N_Vector y, N_Vector dy, N_Vector r, void *udata);

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

  rc->lg = &ofs;

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
  double rtl = 0,
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

int F(realtype t, N_Vector y, N_Vector dy, N_Vector r, void *udata)
{
  ResidualClosure *rc = static_cast<ResidualClosure*>(udata);
  rc->resolve();
  FL(rc->L(), t, y, dy, r, udata);


  /*
  *(rc->lg) << "-- ";
  for(size_t i=0; i<rc->L(); ++i)
  {
    *(rc->lg) << rc->y[i] << ",";
  }
  *(rc->lg) << endl;
  
  *(rc->lg) << "++ ";
  for(size_t i=0; i<rc->L(); ++i)
  {
    *(rc->lg) << rc->ry[i] << ",";
  }
  *(rc->lg) << endl;
  */

  return 0;  
}

int FL(long int /*L*/, realtype t, N_Vector y, N_Vector dy, 
    N_Vector r, void *udata)
{
  ResidualClosure *rc = static_cast<ResidualClosure*>(udata);

  realtype *rv = NV_DATA_P(r);
  rc->y = NV_DATA_P(y);
  rc->dy = NV_DATA_P(dy);
  
  rc->compute(rv, t);

  return 0;  
}
