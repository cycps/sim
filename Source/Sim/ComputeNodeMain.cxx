#include <Cypress/Sim/ComputeNode.hxx>
#include <Cypress/Sim/ResidualClosure.hxx>
#include <RyMPI/runtime.hxx>
#include <iostream>
#include <fstream>
#include <string>

using std::cout;
using std::endl;
using std::ofstream;
using std::to_string;
using namespace cypress;

extern ResidualClosure *rc;

int main(int argc, char **argv)
{
  ofstream ofs{rc->experimentInfo() + to_string(rc->id())};
  ofs << "Cypress Compute Node ... Engage" << endl;

  RyMPI::Runtime rt(&argc, &argv);
  RyMPI::CommInfo ci = rt.commInfo();
  MPI_Comm_dup(ci.comm, &rc->ycomm);
  MPI_Comm_dup(ci.comm, &rc->dycomm);

  rc->init();

  return 0;
}
