#include <Cypress/Compiler/Driver.hxx>
#include <iostream>

using namespace cypress::compile;
using std::cout;
using std::endl;

int main(int argc, char **argv)
{
  Driver driver(argc, argv);
  driver.run();
  /*
  try
  {
  }
  catch(std::exception &e)
  {
    cout << "Shit! -- " << e.what() << endl;
    throw;
    return -1;
  }
  */

}
