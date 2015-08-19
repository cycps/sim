#include <fstream>
#include <Cypress/Control/ControlNode.hxx>
#include <string>
#include <signal.h>

using std::ofstream;
using namespace cypress;
using namespace cypress::control;
using std::endl;
using std::to_string;
using std::string;

struct RotorSpeedController : public Controller
{
  size_t rw_idx;

  size_t wt{6};

  RotorSpeedController(string input, string target) : Controller("rsc")
  {
    setDestination("localhost");
    setTarget(target);
    rw_idx = setInput(input);
  }
    
  void compute() override
  {
    double v = input_frame[rw_idx];
    k_lg << ts() << "rx: v = " << v << endl;
    double u = 5*(wt - v);
    tx(u);
    k_lg << ts() << "tx: u = " << u << endl;
  }
};

RotorSpeedController *rsc = nullptr;

void sigh(int sig)
{
  rsc->k_lg << log("killed by signal: SIGINT(" + to_string(sig) + ")") << endl;
  rsc->io_lg << log("killed by signal: SIGINT(" + to_string(sig) + ")") << endl;
  close(rsc->sockfd);
  exit(1);
}
  

int main(int argc, char **argv)
{
  signal(SIGINT, sigh);

  if(argc != 3) {
    std::cerr << "usage: RotorSpeeController <input> <target>" << std::endl;
    return 1;
  }

  string input = std::string(argv[1]);
  string target = std::string(argv[2]);

  rsc = new RotorSpeedController(input, target);

  rsc->run();
}
