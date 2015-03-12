#include "Cypress/Control/ControlNode.hxx"
#include <iostream>

using namespace cypress;
using namespace cypress::control;

extern Controller *C;

int main()
{
  std::cout << "I am " << C->name << " hear me roar" << std::endl;
}
