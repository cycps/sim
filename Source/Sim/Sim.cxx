#include "Cypress/Sim/Sim.hxx"
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <stdexcept>
#include <sstream>
#include <unordered_map>

using namespace cypress;
using std::vector;
using std::runtime_error;
using std::make_shared;
using std::string;
using std::static_pointer_cast;
using std::stringstream;
using std::unordered_map;

//Cypress::Sim ----------------------------------------------------------------

Sim::Sim( vector<ObjectSP> objects, vector<ControllerSP> controllers,
    ExperimentSP exp) 
  : objects{objects}, controllers{controllers}, exp{exp}
{
  elements.insert(elements.end(), objects.begin(), objects.end());
  elements.insert(elements.end(), controllers.begin(), controllers.end());
}

void Sim::addObjectToSim(ComponentSP c)
{
  EqtnQualifier eqq;
  eqq.setQualifier(c);
  for(auto eqtn: c->element->eqtns)
  {
    auto cpy = eqtn->clone();
    eqq.run(cpy);
    setToZero(cpy);

    for(auto p: c->params)
    {
      string sym_name{c->name->value+"."+p.first->value};
      applyParameter(cpy, sym_name, p.second->value);
    }
      
    psys.push_back(cpy);
  }
}

#include <iostream>
void Sim::addControllerToSim(ComponentSP c)
{
  std::cout << "adding controller " 
            << c->name->value << " :: "
            << c->kind->value << std::endl;

  //vector<SubComponentRefSP> uc = findControlledSubComponents(c);  
}

//TODO: This should be a semantic action
string getControlled(ConnectableSP c)
{
  if(c->neighbor != nullptr) return getControlled(c->neighbor);

  if(c->kind() != Connectable::Kind::SubComponent)
    throw runtime_error{"well fuck"};

  auto x = static_pointer_cast<SubComponentRef>(c);
  return x->name->value + "." + x->subname->value;
}

void Sim::addControllerRefToSim(SubComponentRefSP c)
{
  string under_control = getControlled(c);

  for(auto eqtn: psys)
  {
    liftControlledVars(eqtn, under_control);
  }
}

void Sim::buildSystemEquations()
{
  for(auto c: exp->components)
  {
    if(c->element->kind() == Decl::Kind::Object) addObjectToSim(c);
    //if(c->element->kind() == Decl::Kind::Controller) addControllerToSim(c);
  }

  for(ConnectionSP cx : exp->connections)
  {
    if(isa(cx->from, Connectable::Kind::SubComponent))
    {
      auto sc = static_pointer_cast<SubComponentRef>(cx->from);
      if(isa(sc->component->element, Decl::Kind::Controller))
      {
        addControllerRefToSim(sc); 
      }
    }
  }
}

void Sim::buildPhysics()
{
  buildSystemEquations();
}
 
SimEx Sim::buildSimEx()
{
  SimEx sx{psys.size(), 1e-4, 1e-6}; 
  sx.residualClosureSource = buildResidualClosure();

  return sx;
}

string qdif(string s)
{
  if(boost::starts_with(s, "d_")) return "d";
  return "";
};

string vax(string s)
{
  return s + "_ax";
}

string sysinclude(string file)
{
  return "#include <"+file+">";
}

string use(string name)
{
  return "using " + name + ";";
}

string use_namespace(string name)
{
  return "using namespace " + name + ";";
}

string rcname(ExperimentSP ex)
{
  return ex->name->value + "RC";
}

string Sim::buildResidualClosure()
{
  using std::endl;
  stringstream ss;

  EqtnVarCollector evc;
  for(EquationSP eqtn: psys) evc.run(eqtn);

  ss 
    << sysinclude("Cypress/Sim/ResidualClosure.hxx") << endl
    << sysinclude("cmath") << endl;
  ss << endl;

  ss << use_namespace("cypress") << endl;
  ss << use("std::string") << endl;
  ss << endl;

  ss 
    << "struct " << rcname(exp) << " : ResidualClosure" << endl
    << "{" << endl;

  size_t ax{0};
  for(string sym: evc.vars) 
    ss << "  " 
      << "static constexpr size_t " << vax(sym) << "{" << ax++ << "};" 
      << endl;
  ss << endl;
  
  for(string sym: evc.vars) 
    ss << "  realtype " << sym << "()" << endl
       << "  {" << endl
       << "    return " 
        << qdif(sym) << "yresolve(varmap[" << vax(sym) << "]);" << endl
       << "  }" << endl << endl;

  ss << "  " << "void compute(realtype *r) override" << endl;
  ss << "  " << "{" << endl;
  CxxResidualFuncBuilder rcb;
  size_t idx{0};
  for(EquationSP eqtn: psys) ss << "    " << rcb.run(eqtn, idx++) << endl;
  ss << "  " << "}" << endl;

  ss << "  " << "string experimentInfo() override" << endl
     << "  " << "{" << endl
     << "  " << "  " << "return \"" << exp->name->value << "\";" << endl
     << "  " << "}" << endl;

  ss << "};" << endl << endl;

  ss << rcname(exp) << " *rc = new " << rcname(exp) << ";" << endl << endl;


  return ss.str();
}

//Cypress::CxxResidualFuncBuilder ---------------------------------------------

//Assumes eqtn is already in residual form e.g., 0 = f(x);
string CxxResidualFuncBuilder::run(EquationSP eqtn, size_t idx)
{
  ss.str("");
  //ss << "[this](){ return ";
  ss << "r[" << idx << "] = ";
  eqtn->rhs->accept(*this);
  ss << ";";
  return ss.str();
}

void CxxResidualFuncBuilder::in(AddSP) 
{ 
  ss << " + ";
}

void CxxResidualFuncBuilder::in(SubtractSP) 
{ 
  ss << " - ";
}

void CxxResidualFuncBuilder::in(MultiplySP) 
{ 
  ss << "*";
}

void CxxResidualFuncBuilder::in(DivideSP) 
{ 
  ss << "/";
}

void CxxResidualFuncBuilder::in(SymbolSP s) 
{ 
  string sname = s->value;
  boost::replace_all(sname, ".", "_");
  ss << sname << "()"; 
}

void CxxResidualFuncBuilder::visit(PowSP) 
{ 
  ss << "pow(";
}

void CxxResidualFuncBuilder::in(PowSP) 
{ 
  ss << ",";
}

void CxxResidualFuncBuilder::leave(PowSP) 
{ 
  ss << ")";
}

void CxxResidualFuncBuilder::in(RealSP r) 
{ 
  ss << r->value;
}

void CxxResidualFuncBuilder::visit(DifferentiateSP) 
{ 
  ss << "d_";
}

void CxxResidualFuncBuilder::visit(SubExpressionSP) 
{ 
  ss << "(";
}

void CxxResidualFuncBuilder::leave(SubExpressionSP) 
{ 
  ss << ")";
}

//Cypress::EqtnVarCollector ---------------------------------------------------

void EqtnVarCollector::run(EquationSP eqtn)
{
  eqtn->accept(*this);
}

void EqtnVarCollector::in(SymbolSP s)
{
  string sname = s->value;
  boost::replace_all(sname, ".", "_");
  if(in_derivative) sname = "d_" + sname;
  vars.insert(sname);
}

void EqtnVarCollector::visit(DifferentiateSP)
{
  in_derivative = true;
}

void EqtnVarCollector::leave(DifferentiateSP)
{
  in_derivative = false;
}
