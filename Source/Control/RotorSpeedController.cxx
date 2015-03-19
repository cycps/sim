#include <fstream>
#include <Cypress/Control/ControlNode.hxx>

using std::ofstream;
using namespace cypress;
using namespace cypress::control;
using std::endl;

struct RotorSpeedController : public Controller
{
  RotorSpeedController() : Controller("RotorSpeedController")
  {
    setTarget("rt_act");
  }
    
  void compute() override
  {

  }
};

int main()
{
  RotorSpeedController rsc;
}
