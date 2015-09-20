#include <fstream>
#include <Cypress/Control/ControlNode.hxx>
#include <string>
#include <signal.h>
#include <yaml-cpp/yaml.h>

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
  string config_file;
  string target;
  InputSource input;

  RotorSpeedController(string config_file) : Controller("rsc"), config_file{config_file}
  {
    readConfig();
    std::cout << "w:" << std::endl
              << "  variable: " << input.variable << std::endl
              << "  source: " << input.source << std::endl
              << std::endl;

    std::cout << "tau: " << target << std::endl;
    std::cout << "bye bye" << std::endl;

    exit(1);
    rw_idx = setInput(input.variable);
    setDestination(target);
    setTarget(target);
  }

  void readConfig()
  {
    YAML::Node config = YAML::LoadFile(config_file);
    input = InputSource {
      config["inputs"]["w"]["variable"].as<string>(),
      config["inputs"]["w"]["source"].as<string>()
    };
    target = config["outputs"]["tau"].as<string>();
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

  if(argc != 2) {
    std::cerr << "usage: RotorSpeeController <config_file>" << std::endl;
    return 1;
  }

  //string input = std::string(argv[1]);
  //string target = std::string(argv[2]);

  //rsc = new RotorSpeedController(input, target);
  rsc = new RotorSpeedController(argv[1]);
  rsc->run();
}
