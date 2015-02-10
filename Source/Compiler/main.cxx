#include <Cypress/Compiler/Driver.hxx>
#include <iostream>

using namespace cypress::compile;
using std::cout;
using std::endl;
using std::locale;

int main(int argc, char **argv)
{
  locale::global(locale("en_US.UTF-8"));
  Driver driver(argc, argv);
  driver.run();
}
