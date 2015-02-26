#include "Cypress/Sim/ComputeNode.hxx"
#include "Cypress/Core/Elements.hxx"
#include <boost/algorithm/string/replace.hpp>
#include <sstream>

using std::endl;
using std::string;
using std::ostream;
using std::string;
using std::stringstream;

using namespace cypress;

void localAccessor(string var, string from, size_t i, stringstream &ss)
{
  ss << "  inline realtype " + var + "()" << endl
     << "  {" << endl
     << "    return "<<from<<"["<<i<<"];" << endl
     << "  }" << endl
     << endl;
}

void remoteAccessor(RVar v, size_t i, string mod, string from, stringstream &ss)
{
  ss << "  inline realtype " << mod << v.name << "()" << endl
     << "  {" << endl
     << "    return " << from << "cache["<<i<<"];" << endl
     << "  }" << endl
     << endl;
}

void remoteResolver(RVar v, size_t i, string from, stringstream &ss)
{
  ss << "    "<<from<<"resolve({"
                <<v.coord.px<<","<<v.coord.gx<<","<<v.coord.lx<<"}, "
                << "&" << from << "cache["<<i<<"], "
                << from << "win);" << endl;
}

void controlAccessor(string var, stringstream &ss)
{
  string fname = var;
  boost::replace_all(fname, ".", "_");
  ss << "  inline realtype cx_" + fname + "()" << endl
     << "  {" << endl
     << "    //return cxresolve(hash<string>{}(\""<<var<<"\"));" << endl
     << "    return 2.1;" << endl
     << "  }" << endl
     << endl;
}

string ComputeNode::emitSource()
{
  stringstream ss;

  ss << "#include <Cypress/Sim/ResidualClosure.hxx>" << endl
     << "#include <cmath>" << endl
     << "#include <string>" << endl
     << "#include <array>" << endl
     << "#include <RyMPI/runtime.hxx>" << endl
     << endl;

  ss << "using namespace cypress;" << endl
     << "using std::string;" << endl
     << "using std::hash;" << endl
     << "using std::array;" << endl
     << "using RyMPI::pointerWindow;" << endl
     << endl;

  ss << "struct CNode : public ResidualClosure" << endl;
  ss << "{" << endl;

  ss << "  // Experiment Info -------------------------------------------------"
     << endl
     << "  string experimentInfo() override" << endl
     << "  {" << endl
     << "    return \"" << expInfo << "\";" << endl
     << "  }" << endl
     << endl;

  ss << "  // Local Access Variables ------------------------------------------"
     << endl;

  size_t i{0};
  for(const string &s: vars)
    localAccessor(s, "y", i, ss),
    localAccessor("d_"+s, "dy", i++, ss);

  ss << "  // Remote Access Variables -----------------------------------------"
     << endl;

  ss << "  array<realtype,"<<rvars.size()<<"> ycache, dycache;" << endl
     << endl;

  i=0;
  for(const RVar &v: rvars)
    remoteAccessor(v, i, "", "y", ss),
    remoteAccessor(v, i++, "d_", "dy", ss);

  ss << "  void resolve() override" << endl
     << "  {" << endl;
  i=0;
  ss << "    MPI_Win_fence(0, ywin);" << endl;
  for(const RVar &v: rvars) remoteResolver(v, i++, "y", ss);
  ss << "    MPI_Win_fence(0, ywin);" << endl;
  ss << endl;

  i=0;
  ss << "    MPI_Win_fence(0, dywin);" << endl;
  for(const RVar &v: rvars) remoteResolver(v, i++, "dy", ss);
  ss << "    MPI_Win_fence(0, dywin);" << endl;
  ss << "  }" << endl
     << endl;

  ss << "  // Controll Access Variables ---------------------------------------"
     << endl;
  CVarExtractor cvx;
  for(EquationSP eqtn: eqtns) eqtn->accept(cvx);
  for(string s: cvx.cvars)
    controlAccessor(s, ss);

  ss << "  // Residual Computation --------------------------------------------"
     << endl;

  ss << "  void compute(realtype *r, realtype t) override" << endl
     << "  {" << endl
     << "    //resolveRemotes();" << endl;

  CxxResidualFuncBuilder cxr;
  i=0;
  for(EquationSP eqtn: eqtns) ss << "    " << cxr.run(eqtn, i++) << endl;

  ss << "  }" << endl
     << endl;

  ss << "  // init -----------------------------------------------------------"
     << endl;

  ss << "  void init() override" << endl
     << "  {" << endl;
     
  ss << "    ywin = pointerWindow(y, L(), ycomm);" << endl
     << "    dywin = pointerWindow(dy, L(), dycomm);" << endl;

  for(auto p: initials)
  {
    ss << "    y[" << p.first << "] = " << p.second.v << ";" << endl;
    ss << "    dy[" << p.first << "] = " << p.second.d << ";" << endl;
  }

  ss << "  }" << endl
     << endl;
  
  ss << "  // id -------------------------------------------------------------"
     << endl;
  
  ss << "  size_t id() override" << endl
     << "  {" << endl
     << "    return " << id << ";" << endl
     << "  }" << endl
     << endl;

  ss << "  // L ---------------------------------------------------------------"
     << endl;

  ss << "  size_t L() override" << endl
     << "  {" << endl
     << "    return " << vars.size() << ";" << endl
     << "  }" << endl
     << endl;
  
  ss << "  // N ---------------------------------------------------------------"
     << endl;

  ss << "  size_t N() override" << endl
     << "  {" << endl
     << "    return " << N << ";" << endl
     << "  }" << endl
     << endl;

  ss << "  // ctor -----------------------------------------------------------"
     << endl;
 
  ss << "  CNode()" << endl
     << "  {" << endl
     << "  }" << endl
     << endl;
 
  ss << "};" << endl
     << endl;
  
  ss << "CNode *rc = new CNode;" << endl
     << endl;

  //TODO: you are here-ish .... need to initialize DAE system

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

