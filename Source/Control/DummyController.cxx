#include "Cypress/Control/ControlNode.hxx"
using namespace cypress::control;

struct Dummy : Controller
{
  Dummy() : Controller{"ctrl"} {}
  void compute(realtype*, realtype) override {}
};

Dummy *C = new Dummy;
