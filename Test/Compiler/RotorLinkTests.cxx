#include "gtest/gtest.h"
#include <iostream>
#include <string>

#include <Cypress/Compiler/Driver.hxx>

using std::string;
using std::cout;
using std::endl;

TEST(RotorLink, Basic)
{
  char *cyp_home = getenv("CYPRESS_HOME");
  if(cyp_home == nullptr) 
    FAIL() << "The environment variable CYPRESS_HOME must be set to run tests";

  string
    loc = "nolocation",
    inp = string(cyp_home) + "/Example/RotorControl.cyp";

  const char* args[] = 
  {
    loc.c_str(),
    inp.c_str()
  };
  
  cypress::compile::Driver d{2, const_cast<char**>(args)};
  d.init();
  d.parseInput();
  d.checkSemantics(); 
  d.applyBounds();
  d.buildSim(1);
  //d.buildControlSystem();
  
  cout << "Compute Nodes-----------------------------------------------"
    << endl << endl;

  for(const cypress::sim::ComputeNode &n: d.sim_ex.computeNodes)
    cout << n << endl;
  
  cout << "------------------------------------------------------------"
    << endl;
 
  cout << "Controllers ------------------------------------------------"
    << endl << endl;

  /*
  for(const cypress::control::ControlNode &n: d.ctrlsys->controlNodes)
    cout << n << endl;
    */
  
  cout << "------------------------------------------------------------"
    << endl;

  d.createCypk();

  cypress::ObjectSP rotor = d.decls->objects[0];
  EXPECT_EQ(0ul, rotor->bounds.size());

  cypress::ControllerSP ctrl = d.decls->controllers[0];
  EXPECT_EQ(1ul, ctrl->bounds.size());

  cypress::BoundSP bd = ctrl->bounds[0];
  EXPECT_EQ(cypress::Expression::Kind::Differentiate, bd->lhs->kind());

  /*
  cypress::EqtnPrinter eqp;
  eqp.run(ctrl);
  for(string s : eqp.strings)
    cout << s << endl;
    */
  
}
