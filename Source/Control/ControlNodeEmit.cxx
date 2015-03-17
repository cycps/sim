#include "Cypress/Control/ControlNode.hxx"
#include "Cypress/Core/Elements.hxx"
#include <iostream>
#include <sstream>

using std::string;
using namespace cypress;
using namespace cypress::control;
using std::ostream;
using std::endl;
using std::stringstream;

void ControlNode::emit_ctor() const
{
  *ss << "  //ctor ------------------------------------------------------------"
      << endl;
     
  *ss << "  " << name << "()" << endl
      << "    : Controller{\""<<name<<"\"}" << endl 
      << "  {" << endl
      << "    N = " << compute_vars.size() << ";" << endl
      << "    imapInit();" << endl
      << "    omapInit();" << endl
      << "    resolveInit();" << endl
      << "  }" << endl
      << endl;
}

void ControlNode::emit_imapInit() const
{
  *ss << "  //imapInit --------------------------------------------------------"
      << endl;
     
  *ss << "  void imapInit()" << endl 
      << "  {" << endl;

  for(IOMap iom : inputs)
  {
    *ss << "    std::hash<std::string> hsh{};" << endl;
    *ss << "    imap[hsh(\""<<iom.remote.who<<"\")+"
        <<          "hsh(\""<<iom.remote.what<<"\")]" 
        << " = " << inputIndex(iom.local) << ";"
        << endl;
  }

  *ss << "    a_.buf = vector<vector<CVal>>("<<inputs.size()<<", {});" << endl;
  *ss << "    b_.buf = vector<vector<CVal>>("<<inputs.size()<<", {});" << endl;
  *ss << "    input_frame = vector<double>("<<inputs.size()<<", 0);" << endl;

  *ss << "  }" << endl
      << endl;
}

void ControlNode::emit_omapInit() const
{
  *ss << "  //omapInit -------------------------------------------------------"
      << endl;

  *ss << "  void omapInit()" << endl
      << "  {" << endl
      << "    std::hash<std::string> hsh{};" << endl;

  for(IOMap iom : outputs)
  {
    *ss << "    omap[" << computeIndex(iom.local) << "] = " 
        << "{hsh(\""<<iom.remote.who<<"\"), hsh(\""<<iom.remote.what<<"\")};"
        << endl;
  }

  *ss << " }" << endl
      << endl;
}

void ControlNode::emit_resolveInit() const
{
  *ss << "  //resolveInit -----------------------------------------------------"
      << endl;
     
  *ss << "  void resolveInit()" << endl 
      << "  {" << endl;

  *ss << "    resolvers = " 
      << "vector<FrameVarResolver>(imap.size(), UseLatestArrival);"
      << endl;


  *ss << "  }" << endl
      << endl;

}

void ControlNode::emit_accessors() const
{
  *ss << "  //compute accessors -----------------------------------------------"
      << endl;

  for(const string &s: compute_vars)
  {
    *ss << "  realtype " << s << "()" << endl
        << "  {" << endl
        << "    return y[" << computeIndex(s) << "];" << endl
        << "  }" << endl
        << endl;
    
    *ss << "  realtype d_" << s << "()" << endl
        << "  {" << endl
        << "    return dy[" << computeIndex(s) << "];" << endl
        << "  }" << endl
        << endl;
  }
  
  *ss << "  //compute accessors -----------------------------------------------"
      << endl;

  for(const IOMap iom: inputs)
  {
    *ss << "  realtype in_" << iom.local << "()" << endl
        << "  {" << endl
        << "    return input_frame[" << inputIndex(iom.local) << "];" << endl
        << "  }" << endl
        << endl;
  }

}

void ControlNode::emit_residualFunc() const
{
  *ss << "  //control action --------------------------------------------------"
      << endl;

  *ss << "  void compute(realtype *r, realtype t) override" << endl
      << "  {" << endl;

  CxxResidualFuncBuilder cxr;
  cxr.qnames = false;
  size_t i{0};
  for(EquationSP eq: eqtns)
    *ss << "    " << cxr.run(source, eq, i++) << endl;

  *ss << "  }" << endl
      << endl;
}

string ControlNode::emitSource() const
{
  *ss << "#include \"Cypress/Control/ControlNode.hxx\"" << endl
      << "#include \"Cypress/Core/Utility.hxx\"" << endl
      << "#include <vector>" << endl
      << endl;

  *ss << "using namespace cypress::control;" << endl
      << "using std::vector;" << endl
      << endl;

  *ss << "struct " << name << " : Controller" << endl
      << "{" << endl;

  emit_ctor();
  emit_imapInit();
  emit_omapInit();
  emit_resolveInit();
  emit_accessors();
  emit_residualFunc();

  *ss << "};" << endl << endl;

  *ss << name << " *C = new " << name << ";" << endl;

  return ss->str();
}

ostream & cypress::control::operator << (ostream &o, const ControlNode &n)
{
  o << "name=" << n.name << endl;

  EqtnPrinter eqp;
  o << "[eqtn]" << endl;
  for(EquationSP eq: n.eqtns) eqp.run(eq);
  for(const string &s: eqp.strings) o << "  " << s << endl;

  o << "[input]" << endl;
  for(const IOMap &iom: n.inputs)
    o << iom.local << " <-- " 
      << "(" << iom.remote.who << "," << iom.remote.what << ")"
      << endl;

  o << "[output]" << endl;
  for(const IOMap &iom: n.outputs)
    o << iom.local << " --> " 
      << "(" << iom.remote.who << "," << iom.remote.what << ")"
      << endl;

  o << "[compute]" << endl;
  for(const string &s: n.compute_vars)
    o << s << endl;

  return o;
}
