#include <Cypress/Sim/ResidualClosure.hxx>

using std::string;
using namespace cypress;

struct DummyResid : public ResidualClosure
{
  void compute(realtype*) override {}
  string experimentInfo() override { return "DummyExperiment"; }
};

DummyResid *rc = new DummyResid;
