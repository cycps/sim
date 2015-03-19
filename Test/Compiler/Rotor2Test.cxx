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
    inp = string(cyp_home) + "/Example/RotorControl2.cyp";

  const char* args[] = 
  {
    loc.c_str(),
    inp.c_str()
  };
  
  cypress::compile::Driver d{2, const_cast<char**>(args)};
  d.init();
  d.parseInput();

}
