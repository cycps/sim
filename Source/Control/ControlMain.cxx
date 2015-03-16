#include "Cypress/Control/ControlNode.hxx"
#include <csignal>
#include <string>
#include <iostream>

using namespace cypress::control;
using cypress::log;
using std::signal;
using std::cout;
using std::endl;
using std::to_string;

extern Controller *C;

void sigh(int sig)
{
  C->k_lg << log("killed by signal: SIGINT(" + to_string(sig) + ")") << endl;
  C->io_lg << log("killed by signal: SIGINT(" + to_string(sig) + ")") << endl;
  cout << endl;
  exit(1);
}

int F(realtype t, N_Vector y, N_Vector dy, N_Vector r, void*)
{
  C->y = NV_DATA_S(y);
  C->dy = NV_DATA_S(dy);

  realtype *rv = NV_DATA_S(r);

  C->compute(rv, t);

  return 0;
}

int main()
{
  signal(SIGINT, sigh);
  C->F = F;
  C->run();
}
