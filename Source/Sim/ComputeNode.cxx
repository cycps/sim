#include "Cypress/Sim/ComputeNode.hxx"
#include "Cypress/Core/Elements.hxx"
#include <sstream>

using std::endl;
using std::string;
using std::ostream;
using std::string;
using std::stringstream;

using namespace cypress;

void localAccessor(string var, string from, size_t i, stringstream &ss)
{
  ss << "  static inline realtype " + var + "()" << endl
      << "  {" << endl
      << "    return "<<from<<"["<< i<<"]" << endl
      << "  }" << endl
      << endl;
}

void remoteAccessor(RVar v, string mod, string from, stringstream &ss)
{
  ss << "  static inline realtype " << mod << v.name << "()" << endl
      << "  {" << endl
      << "    return "<<from<<"resolve({"
                <<v.coord.px<<","<<v.coord.gx<<","<<v.coord.lx<<"});" << endl
      << "  }" << endl
      << endl;
}

string ComputeNode::emitSource()
{
  stringstream ss;

  ss << "#include <Cypress/Sim/ResidualClosure.hxx>" << endl
     << "#include <cmath>" << endl
     << endl;

  ss << "using namespace cypress;" << endl
     << endl;

  ss << "struct CNode : public ResidualClosure" << endl;
  ss << "{" << endl;

  ss << "  // Local Access Variables ------------------------------------------"
     << endl;
  size_t i{0};
  for(const string &s: vars)
    localAccessor(s, "y", i, ss),
    localAccessor("d_"+s, "dy", i++, ss);

  ss << "  // Remote Access Variables -----------------------------------------"
     << endl;
  for(const RVar &v: rvars)
    remoteAccessor(v, "", "y", ss),
    remoteAccessor(v, "d_", "dy", ss);

  ss << "  // Residual Computation --------------------------------------------"
     << endl;

  ss << "  compute(realtype *r) override" << endl
     << "  {" << endl;

  CxxResidualFuncBuilder cxr;
  i=0;
  for(EquationSP eqtn: eqtns) ss << "    " << cxr.run(eqtn, i++) << endl;

  ss << "  }" << endl
     << endl;

  ss << "};" << endl
     << endl;

  return ss.str();
}

ostream & cypress::operator << (ostream &o, const ComputeNode &n)
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

