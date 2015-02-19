#include <Cypress/Sim/ComputeNode.hxx>
#include <Cypress/Sim/ResidualClosure.hxx>
#include <iostream>

using std::cout;
using std::endl;
using namespace cypress;

extern ResidualClosure *rc;

int main()
{
  cout << "Cypress Compute Node ... Engage" << endl;
  cout << rc->experimentInfo() << endl;
  return 0;
}
