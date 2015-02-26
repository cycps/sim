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
  ofstream ofs{rc->experimentInfo() + to_string(rc->id())};
  ofs << "Cypress Compute Node ... Engage" << endl;

  RyMPI::Runtime rt(&argc, &argv);
  RyMPI::CommInfo ci = rt.commInfo();
  MPI_Comm_dup(ci.comm, &rc->ycomm);
  MPI_Comm_dup(ci.comm, &rc->dycomm);

  //init residual closure nvectors
  rc->nv_y = N_VNew_Parallel(rc->ycomm, rc->L(), 47);
  rc->nv_dy = N_VNew_Parallel(rc->dycomm, rc->L(), 47);

  rc->y = NV_DATA_P(rc->nv_y);
  rc->dy = NV_DATA_P(rc->nv_dy);
  
  rc->init();

  return 0;
}
