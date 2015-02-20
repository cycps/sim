#include "Cypress/Sim/ComputeNode.hxx"
#include "Cypress/Core/Elements.hxx"

using std::endl;
using std::string;

std::ostream & cypress::operator << (std::ostream &o, const ComputeNode &n)
{
  o << "id=" << n.id << endl;

  o << "[var]" << endl;
  for(string s: n.vars)
    o << "  " << s << endl;

  o << "[rvar]" << endl;
  for(RVar r: n.rvars)
    o << "  " << r.name << " (" << r.coord.px << ")" << endl;

  EqtnPrinter eqp;
  o << "[eqtn]" << endl;
  for(EquationSP e: n.eqtns) eqp.run(e);
  for(auto eq_str : eqp.strings) o << "  " << eq_str << endl;

  return o;
}
