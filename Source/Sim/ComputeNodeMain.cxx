#include <Cypress/Sim/ComputeNode.hxx>
#include <Cypress/Sim/ResidualClosure.hxx>
#include <RyMPI/runtime.hxx>
#include <iostream>

using std::cout;
using std::endl;
using namespace cypress;

extern ResidualClosure *rc;

int main(int argc, char **argv)
{
  cout << "Cypress Compute Node ... Engage" << endl;
  cout << rc->experimentInfo() << endl;

  RyMPI::Runtime rt(&argc, &argv);
  RyMPI::CommInfo ci = rt.commInfo();
  MPI_Comm_dup(ci.comm, &rc->ycomm);
  MPI_Comm_dup(ci.comm, &rc->dycomm);

  return 0;
}
