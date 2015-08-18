#include <Cypress/Sim/Simutron.hxx>

using std::string;
using namespace cypress;
using namespace cypress::sim;

struct DummyResid : public Simutron
{
  using Simutron::Simutron;
  void compute(realtype*, realtype) override {}
  virtual std::string dataHeader() override { return ""; }
  void resolve() override {}
  string experimentInfo() override { return "DummyExperiment"; }
  void init() override {}
  size_t id() override { return 47; }
  size_t L() override { return 74; }
  size_t N() override { return 666; }
  size_t cN() override { return 666; }
};

DummyResid *rc = new DummyResid("dummy");
