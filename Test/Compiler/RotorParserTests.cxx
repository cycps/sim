#include "gtest/gtest.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include <Cypress/Compiler/Driver.hxx>

using std::string;
using std::vector;


TEST(Parser, Rotor)
{
  std::locale::global(std::locale("en_US.UTF-8"));

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

  ASSERT_EQ(1ul, d.decls->objects.size());
  ASSERT_EQ(1ul, d.decls->controllers.size());
  ASSERT_EQ(1ul, d.decls->experiments.size());

  //Rotor object structure
  cypress::ObjectSP rotor = d.decls->objects[0];

  EXPECT_EQ("Rotor", rotor->name->value);
  ASSERT_EQ(1ul, rotor->params.size());
  EXPECT_EQ("H", rotor->params[0]->value);
  ASSERT_EQ(3ul, rotor->eqtns.size());
  
  //RotorSpeedController
  cypress::ControllerSP rsp = d.decls->controllers[0];

  EXPECT_EQ("RotorSpeedController", rsp->name->value);
  ASSERT_EQ(2ul, rsp->eqtns.size());


  //RotorControl experiment
  cypress::ExperimentSP xsp = d.decls->experiments[0];

  EXPECT_EQ("RotorControl", xsp->name->value);
  EXPECT_EQ(4ul, xsp->components.size());
  EXPECT_EQ(5ul, xsp->connections.size());

  //rotor component instance
  cypress::ComponentSP rc = xsp->components[0];
  EXPECT_EQ("rotor", rc->name->value);
  ASSERT_EQ(1ul, rc->params.size());
  ASSERT_EQ(6ul, rc->initials.size());

  //Checking argument parsing
  EXPECT_EQ(2.5, rc->parameterValue("H")->value);
  EXPECT_EQ(0.0, rc->initialValue("θ"));
  EXPECT_EQ(0.0, rc->initialValue("θ", cypress::VarRef::Kind::Derivative));
  EXPECT_EQ(2.1, rc->initialValue("a"));
  EXPECT_EQ(2.1, rc->initialValue("τ"));
  EXPECT_EQ(0.0, rc->initialValue("ω", cypress::VarRef::Kind::Derivative));
}
