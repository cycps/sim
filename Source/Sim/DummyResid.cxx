#include <Cypress/Sim/ResidualClosure.hxx>

using std::string;
using namespace cypress;

struct DummyResid : public ResidualClosure
{
  void compute(realtype*) override {}
  string experimentInfo() override { return "DummyExperiment"; }
  void init() override {}
  size_t id() override { return 47; }
};

DummyResid *rc = new DummyResid;
