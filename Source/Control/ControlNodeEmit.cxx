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
      << "  }" << endl
      << endl;
}

void ControlNode::emit_imapInit() const
{
  *ss << "  //imapInit --------------------------------------------------------"
      << endl;
     
  *ss << "  void imapInit()" << endl 
      << "  {" << endl;

      int idx{0};
      for(IOMap iom : inputs)
      {
        *ss << "    std::hash<std::string> hsh{};" << endl;
        *ss << "    imap[hsh(" 
            << "\"" << iom.remote.who + "." << iom.remote.what << "\"" 
            << ")] = " << idx << ";"
            << endl;
      }


  *ss << "  }" << endl
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

  size_t i{0};
  for(const string &s: compute_vars)
  {
    *ss << "  realtype " << s << "()" << endl
        << "  {" << endl
        << "    return y[" << i << "];" << endl
        << "  }" << endl
        << endl;
    
    *ss << "  realtype d_" << s << "()" << endl
        << "  {" << endl
        << "    return dy[" << i << "];" << endl
        << "  }" << endl
        << endl;
  }
  
  *ss << "  //compute accessors -----------------------------------------------"
      << endl;

  i=0;
  for(const IOMap iom: inputs)
  {
    *ss << "  realtype in_" << iom.local << "()" << endl
        << "  {" << endl
        << "    return input_frame[" << i << "];" << endl
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
      << "#include <vector>" << endl
      << endl;

  *ss << "using namespace cypress::control;" << endl
      << "using std::vector;" << endl
      << endl;

  *ss << "struct " << name << " : Controller" << endl
      << "{" << endl;

  emit_ctor();
  emit_imapInit();
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
