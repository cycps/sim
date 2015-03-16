#include "Cypress/Control/ControlNode.hxx"
using namespace cypress::control;

struct Dummy : Controller
{
  Dummy() : Controller{"ctrl"} {}
};

Dummy *C = new Dummy;
