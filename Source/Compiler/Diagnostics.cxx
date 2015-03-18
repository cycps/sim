#include "Cypress/Compiler/Diagnostics.hxx"
#include <iostream>
#include <sstream>

using std::ostream;
using std::stringstream;
using namespace cypress;
using namespace cypress::compile;
using std::string;
using std::endl;

// Diagnostics ================================================================

bool DiagnosticReport::catastrophic()
{
  for(const Diagnostic d: diagnostics)
    if(d.level == Diagnostic::Level::Error)
      return true;

  return false;
}

const char* CompilationError::what() const noexcept
{
  stringstream ss;
  ss << report;
  what_ = ss.str();
  return what_.c_str();
}

ostream& cypress::compile::operator<<(ostream &o, const Diagnostic &d)
{
  string sev;
  switch(d.level)
  {
    case Diagnostic::Level::Error: sev = "Error"; break;
    case Diagnostic::Level::Warning: sev = "Warning"; break;
    case Diagnostic::Level::Info: sev = "Info"; break;
  }
  o << "[" << sev << "]" 
    << "(" << d.line+1 << ":" << d.column+1 << ") " 
    << d.message << endl;
  
  return o;
}

ostream& cypress::compile::operator<<(ostream &o, const DiagnosticReport &dr)
{
  for(Diagnostic diag : dr.diagnostics)
    if(diag.level <= dr.level)
      o << diag;

  return o;
}
