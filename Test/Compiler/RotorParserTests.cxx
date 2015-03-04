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
  EXPECT_EQ(2ul, rotor->line);
  EXPECT_EQ(0ul, rotor->column);
  EXPECT_EQ(2ul, rotor->name->line);
  EXPECT_EQ(7ul, rotor->name->column);

  EXPECT_EQ("Rotor", rotor->name->value);
  ASSERT_EQ(1ul, rotor->params.size());
  EXPECT_EQ("H", rotor->params[0]->value);
  ASSERT_EQ(3ul, rotor->eqtns.size());
  
  //RotorSpeedController
  cypress::ControllerSP rsp = d.decls->controllers[0];

  EXPECT_EQ("RotorSpeedController", rsp->name->value);
  EXPECT_EQ(7ul, rsp->line);
  EXPECT_EQ(0ul, rsp->column);
  EXPECT_EQ(7ul, rsp->name->line);
  EXPECT_EQ(11ul, rsp->name->column);
  ASSERT_EQ(2ul, rsp->eqtns.size());


  //RotorControl experiment
  cypress::ExperimentSP xsp = d.decls->experiments[0];

  EXPECT_EQ("RotorControl", xsp->name->value);
  EXPECT_EQ(4ul, xsp->components.size());
  EXPECT_EQ(5ul, xsp->connections.size());
  EXPECT_EQ(11ul, xsp->line);
  EXPECT_EQ(0ul, xsp->column);
  EXPECT_EQ(11ul, xsp->name->line);
  EXPECT_EQ(11ul, xsp->name->column);

  //rotor component instance
  cypress::ComponentSP rc = xsp->components[0];
  EXPECT_EQ("rotor", rc->name->value);
  EXPECT_EQ("Rotor", rc->kind->value);
  //not until smea
  //EXPECT_EQ(cypress::Decl::Kind::Object, rc->element->kind());
  EXPECT_EQ(nullptr, rc->element);

  EXPECT_EQ(12ul, rc->line);
  EXPECT_EQ(2ul, rc->column);

  EXPECT_EQ(12ul, rc->kind->line);
  EXPECT_EQ(2ul, rc->kind->column);
  
  EXPECT_EQ(12ul, rc->name->line);
  EXPECT_EQ(8ul, rc->name->column);

  ASSERT_EQ(1ul, rc->params.size());
  ASSERT_EQ(6ul, rc->initials.size());

  //Checking argument parsing
  cypress::RealSP H = rc->parameterValue("H");
  EXPECT_EQ(2.5, H->value);
  EXPECT_EQ(12ul, H->line);
  EXPECT_EQ(14ul, H->column);

  cypress::RealSP ω = rc->initialValue("ω");
  EXPECT_EQ(0.0, ω->value);
  EXPECT_EQ(12ul, ω->line);
  EXPECT_EQ(21ul, ω->column);

  cypress::RealSP θ = rc->initialValue("θ");
  EXPECT_EQ(0.0, θ->value);
  EXPECT_EQ(12ul, θ->line);
  EXPECT_EQ(27ul, θ->column);

  cypress::RealSP dθ = rc->initialValue("θ", cypress::VarRef::Kind::Derivative);
  EXPECT_EQ(0.0, dθ->value);
  EXPECT_EQ(12ul, dθ->line);
  EXPECT_EQ(33ul, dθ->column);

  cypress::RealSP a = rc->initialValue("a");
  EXPECT_EQ(2.1, a->value);
  EXPECT_EQ(12ul, a->line);
  EXPECT_EQ(40ul, a->column);
  
  cypress::RealSP τ = rc->initialValue("τ");
  EXPECT_EQ(2.1,  τ->value);
  EXPECT_EQ(12ul, τ->line);
  EXPECT_EQ(47ul, τ->column);

  cypress::RealSP dω = rc->initialValue("ω", cypress::VarRef::Kind::Derivative);
  EXPECT_EQ(0.0, dω->value);
  EXPECT_EQ(12ul, dω->line);
  EXPECT_EQ(55ul, dω->column);

  //ctrl component instance
  cypress::ComponentSP ctrl = xsp->components[1];
  EXPECT_EQ("ctrl", ctrl->name->value);
  EXPECT_EQ("RotorSpeedController", ctrl->kind->value);
  EXPECT_EQ(13ul, ctrl->line);
}
