#include "Cypress/Control/ControlNode.hxx"
#include <csignal>
#include <string>
#include <iostream>

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

using namespace cypress::control;
using cypress::log;
using std::signal;
using std::cout;
using std::cerr;
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

int main(int argc, char **argv)
{
  if(argc < 2)
  {
    C->k_lg << log("Attempt to start controller without middleware target")
            << endl;
    C->io_lg << log("Attempt to start controller without middleware target")
            << endl;
    cerr << "usage: controller <middlware-ip-addr>" << endl;
    exit(1);
  }

  bzero(&C->mwaddr, sizeof(C->mwaddr));
  C->mwaddr.sin_family = AF_INET;
  C->mwaddr.sin_port = htons(7474);
  int err = inet_pton(AF_INET, argv[1], &C->mwaddr.sin_addr);

  if(err < 0)
  {
    C->k_lg << log("Attempt to start controller with bad middleware target")
            << endl;
    C->io_lg << log("Attempt to start controller with bad middleware target")
            << endl;
    cerr << "invalid middlware target address" << endl;
    exit(1);
  }

  signal(SIGINT, sigh);
  C->F = F;
  C->run();
}
