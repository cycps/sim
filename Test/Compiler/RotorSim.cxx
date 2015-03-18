#include "gtest/gtest.h"
#include <iostream>
#include <string>

#include <Cypress/Compiler/Driver.hxx>

using std::string;
using std::cout;
using std::endl;
using namespace cypress;

TEST(RotorLink, Basic)
{
  char *cyp_home = getenv("CYPRESS_HOME");
  if(cyp_home == nullptr) 
    FAIL() << "The environment variable CYPRESS_HOME must be set to run tests";

  string
    loc = "nolocation",
    inp = string(cyp_home) + "/Example/RotorSim.cyp";

  const char* args[] = 
  {
    loc.c_str(),
    inp.c_str()
  };
  
  compile::Driver d{2, const_cast<char**>(args)};
  d.init();
  d.parseInput();
  d.checkSemantics(); 

  ObjectSP rtr = d.decls->objects[0];

  cout << rtr->name->value << endl;
  EqtnPrinter eqp;
  for(EquationSP eq: rtr->eqtns)
    eqp.run(eq);

  for(string s : eqp.strings)
    cout << "  " << s << endl;

  cout << endl;

  SimulationSP sim = d.decls->experiments[0];
  cout << sim->name->value << endl;

  for(ComponentSP c: sim->components)
  {
    cout << "  " << c->name->value << "::" << c->kind->value << endl;
  }

  auto showCx = [](ConnectableSP c) -> string
  {
    if(c->kind() == Connectable::Kind::Component)
      return std::static_pointer_cast<ComponentRef>(c)->name->value;

    if(c->kind() == Connectable::Kind::SubComponent)
    {
      auto sc = std::static_pointer_cast<SubComponentRef>(c);
      return sc->name->value + "." + sc->subname->value;
    }
    return "derp";
  };

  for(ConnectionSP c: sim->connections)
  {
    cout << "  " << showCx(c->from) << " ~ " << showCx(c->to) << endl;
  }
  
  d.buildSim(1);
  d.createCypk();

  /*
  d.applyBounds();
  d.buildControlSystem();
  
  cout << "Compute Nodes-----------------------------------------------"
    << endl << endl;

  for(const cypress::sim::ComputeNode &n: d.sim_ex.computeNodes)
    cout << n << endl;
  
  cout << "------------------------------------------------------------"
    << endl;
 
  cout << "Controllers ------------------------------------------------"
    << endl << endl;

  for(const cypress::control::ControlNode &n: d.ctrlsys->controlNodes)
    cout << n << endl;
  
  cout << "------------------------------------------------------------"
    << endl;


  cypress::ObjectSP rotor = d.decls->objects[0];
  EXPECT_EQ(0ul, rotor->bounds.size());

  cypress::ControllerSP ctrl = d.decls->controllers[0];
  EXPECT_EQ(1ul, ctrl->bounds.size());

  cypress::BoundSP bd = ctrl->bounds[0];
  EXPECT_EQ(cypress::Expression::Kind::Differentiate, bd->lhs->kind());
  */

  /*
  cypress::EqtnPrinter eqp;
  eqp.run(ctrl);
  for(string s : eqp.strings)
    cout << s << endl;
    */
  
}
