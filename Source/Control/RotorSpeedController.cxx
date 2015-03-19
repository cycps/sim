#include <fstream>
#include <Cypress/Control/ControlNode.hxx>

using std::ofstream;
using namespace cypress;
using namespace cypress::control;
using std::endl;

struct RotorSpeedController : public Controller
{
  size_t rw_idx;

  RotorSpeedController() : Controller("RotorSpeedController")
  {
    setDestination("localhost");
    setTarget("rt_act");
    rw_idx = setInput("rotor_w");
  }
    
  void compute() override
  {

  }
};

int main()
{
  RotorSpeedController rsc;
  rsc.run();
}
