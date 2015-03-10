#include "Cypress/Control/ControlNode.hxx"
#include "Cypress/Core/Elements.hxx"
#include <iostream>

using std::string;
using namespace cypress;
using namespace cypress::control;
using std::ostream;
using std::endl;

string ControlNode::emitSource() const
{
  return "";
}

ostream & cypress::control::operator << (ostream &o, const ControlNode &n)
{
  o << "name=" << n.name << endl;

  EqtnPrinter eqp;
  o << "[eqtn]" << endl;
  for(EquationSP eq: n.eqtns) eqp.run(eq);
  for(const string &s: eqp.strings) o << "  " << s << endl;

  return o;
}
