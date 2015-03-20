#include <fstream>
#include <Cypress/Control/ControlNode.hxx>
#include <string>

using std::ofstream;
using namespace cypress;
using namespace cypress::control;
using std::endl;
using std::to_string;

struct RotorSpeedController : public Controller
{
  size_t rw_idx;

  size_t wt{6};

  RotorSpeedController() : Controller("rsc")
  {
    setDestination("localhost");
    setTarget("rt_act");
    rw_idx = setInput("rotor_w");
  }
    
  void compute() override
  {
    double v = input_frame[rw_idx];
    double u = wt - v;
    tx(u);
    k_lg << ts() << "tx: u = " << u << endl;
  }
};

RotorSpeedController rsc;

void sigh(int sig)
{
  rsc.k_lg << log("killed by signal: SIGINT(" + to_string(sig) + ")") << endl;
  rsc.io_lg << log("killed by signal: SIGINT(" + to_string(sig) + ")") << endl;
  close(rsc.sockfd);
  exit(1);
}
  

int main()
{
  signal(SIGINT, sigh);
  rsc.run();
}
