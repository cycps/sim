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

void ControlNode::emitCtor() const
{
  *ss << "  //ctor ------------------------------------------------------------"
    << endl;
     
  *ss << "  " << name << "() : Controller{\""<<name<<"\"}" << endl 
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


  *ss << "  }" << endl;
}

string ControlNode::emitSource() const
{
  *ss << "#include \"Cypress/Control/ControlNode.hxx\"" << endl;

  *ss << "using namespace cypress;" << endl
     << "using namespace cypress::control;" << endl
     << endl;

  *ss << "struct " << name << " : Controller" << endl
     << "{" << endl;

  emitCtor();

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

  return o;
}
