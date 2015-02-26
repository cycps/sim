#include <Cypress/Sim/ResidualClosure.hxx>

using std::string;
using namespace cypress;

struct DummyResid : public ResidualClosure
{
  void compute(realtype*) override {}
  void resolve() override {}
  string experimentInfo() override { return "DummyExperiment"; }
  void init() override {}
  size_t id() override { return 47; }
  size_t L() override { return 74; }
  size_t N() override { return 666; }
};

DummyResid *rc = new DummyResid;
