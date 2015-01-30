#include <Cypress/Compiler/Driver.hxx>
#include <iostream>

using namespace cypress::compile;
using std::cout;
using std::endl;

int main(int argc, char **argv)
{
  Driver driver(argc, argv);
  try
  {
    driver.run();
  }
  catch(std::exception &e)
  {
    cout << e.what() << endl;
    return -1;
  }

}
