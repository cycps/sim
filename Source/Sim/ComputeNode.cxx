#include "Cypress/Sim/ComputeNode.hxx"
#include "Cypress/Core/Elements.hxx"
#include <boost/algorithm/string/replace.hpp>
#include <sstream>
#include <stdexcept>

using std::endl;
using std::string;
using std::ostream;
using std::string;
using std::stringstream;
using std::runtime_error;

using namespace cypress;
using namespace cypress::sim;

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
  ss << "  inline realtype " << mod << v.var->qname() << "()" << endl
     << "  {" << endl
     << "    return r" << from << "["<<i<<"];" << endl
     << "  }" << endl
     << endl;
}

void remoteResolver(RVar v, size_t i, string from, stringstream &ss)
{
  ss << "    "<<from<<"resolve({"
                <<v.coord.px<<","<<v.coord.gx<<","<<v.coord.lx<<"}, "
                << "&r" << from << "["<<i<<"], "
                << from << "win);" << endl;
}

void controlAccessor(string var, stringstream &ss, size_t i)
{
  string fname = var;
  boost::replace_all(fname, ".", "_");
  ss << "  inline realtype " + fname + "_cx()" << endl
     << "  {" << endl
     << "    return c["<<i<<"];" << endl
     << "  }" << endl
     << endl;
}

string ComputeNode::emitSource()
{
  stringstream ss;

  ss << "#include <Cypress/Sim/Simutron.hxx>" << endl
     << "#include <cmath>" << endl
     << "#include <string>" << endl
     << "#include <array>" << endl
     << "#include <RyMPI/runtime.hxx>" << endl
     << endl;

  ss << "using namespace cypress;" << endl
     << "using namespace cypress::sim;" << endl
     << "using std::string;" << endl
     << "using std::hash;" << endl
     << "using std::array;" << endl
     << "using RyMPI::pointerWindow;" << endl
     << endl;

  ss << "struct CNode : public Simutron" << endl;
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

  size_t i{0}, ci{0};
  for(const VarRefSP v: vars)
  {
    string name = v->component->name->value + "_" + v->name;
    localAccessor(name, "y", i, ss);
    localAccessor("d_"+name, "dy", i++, ss);
    if(v->component->element->kind() == Element::Kind::Actuator)
    {
      controlAccessor(name, ss, ci++);
    }
  }

  ss << "  // Remote Access Variables -----------------------------------------"
     << endl;

  //ss << "  array<realtype,"<<rvars.size()<<"> ycache, dycache;" << endl
  //   << endl;

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

  /*
  ss << "  // Controll Access Variables ---------------------------------------"
     << endl;
  auto cvx = VarExtractorFactory::CVarExtractor();
  for(auto p: eqtns) cvx.run(p.first, p.second);
  for(auto v: cvx.vars)
    controlAccessor(v->component->name->value + "_" + v->name, ss);
    */

  ss << "  // Residual Computation --------------------------------------------"
     << endl;

  ss << "  void compute(realtype *r, realtype t) override" << endl
     << "  {" << endl
     << "    //resolveRemotes();" << endl;

  CxxResidualFuncBuilder cxr;
  i=0;
  for(auto p: eqtns) ss << "    " << cxr.run(p.first, p.second, i++) << endl;

  ss << "  }" << endl
     << endl;

  ss << "  // init -----------------------------------------------------------"
     << endl;

  ss << "  void init() override" << endl
     << "  {" << endl;
    
  /*
  ss << "    ywin = pointerWindow(y, L(), ycomm);" << endl
     << "    dywin = pointerWindow(dy, L(), dycomm);" << endl;
     */

  for(auto p: initials)
  {
    ss << "    y[" << p.first << "] = " << p.second.v << ";" << endl;
    ss << "    dy[" << p.first << "] = " << p.second.d << ";" << endl;
  }
  for(size_t i=0; i<cN; ++i)
  {
    ss << "    c["<<i<<"] = 0;" << endl;
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
  
  ss << "  size_t cN() override" << endl
     << "  {" << endl
     << "    return " << cN << ";" << endl
     << "  }" << endl
     << endl;

  ss << "  // ctor -----------------------------------------------------------"
     << endl;
 
  ss << "  CNode(string name) : Simutron(name)" << endl
     << "  {" << endl
     << "    ry = (realtype*)malloc(sizeof(realtype)*" << vars.size() << ");" 
     <<      endl
     << "    rdy = (realtype*)malloc(sizeof(realtype)*" << vars.size() << ");" 
     <<      endl
     << "    hash<string> hsh{};" << endl
     << endl;

  for(VarRefSP v: vars)
  {
    if(v->component->element->kind() != Element::Kind::Actuator) continue;

    ss << "    cmap[hsh(\""<<v->component->name->value<<"\")];" << endl
       << endl;
  }

  for(SensorAttributesSP sens: sensors)
  {
    ss << "    "
       << "sensorManager.add({"
       << "hsh(\"" << sens->target->qname() << "\"), "
       << varidx(sens->target) << ", "
       << sens->rate << ", "
       << "sensorSA(\"" << sens->destination << "\")});" << endl
       << endl;
  }

  ss << "    startControlListener();" << endl
     << "  }" << endl
     <<    endl;
 
  ss << "};" << endl
     << endl;
  
  ss << "CNode *rc = new CNode(\"cnode"<<id<<"\");" << endl
     << endl;

  return ss.str();
}

size_t ComputeNode::varidx(VarRefSP v)
{
  auto it =
    find_if(vars.begin(), vars.end(),
        [v](VarRefSP x)
        {
          return v->qname() == x->qname();
        });

  if(it != vars.end()) return std::distance(vars.begin(), it);

  throw runtime_error{"Unkown var " + v->qname()};
}

ostream & cypress::sim::operator << (ostream &o, const ComputeNode &n)
{
  o << "id=" << n.id << endl;

  o << "[var]" << endl;
  for(VarRefSP v: n.vars)
    o << "  " << v->component->name->value << "_" << v->name << endl;

  o << "[rvar]" << endl;
  for(RVar r: n.rvars)
    o << "  " << r.var->qname() << " (" << r.coord.px << ")" << endl;

  EqtnPrinter eqp;
  o << "[eqtn]" << endl;
  for(auto p: n.eqtns) eqp.run(p.second);
  for(auto eq_str : eqp.strings) o << "  " << eq_str << endl;

  o << "[inital]" << endl;
  for(auto p: n.initials) 
    o << n.vars[p.first]->component->name->value << "_"
      << n.vars[p.first]->name << " --> " 
      << "{" << p.second.v << "," << p.second.d << "}" << endl;


  return o;
}

