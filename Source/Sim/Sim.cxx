#include "Cypress/Sim/Sim.hxx"
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <stdexcept>
#include <sstream>
#include <unordered_map>
#include <algorithm>
#include <array>
#include <iostream>

using namespace cypress;
using namespace cypress::sim;
using std::vector;
using std::runtime_error;
using std::make_shared;
using std::string;
using std::static_pointer_cast;
using std::stringstream;
using std::unordered_map;
using std::find_if;
using std::to_string;
using std::pair;
using std::array;

//Cypress::Sim ----------------------------------------------------------------

Sim::Sim( vector<ObjectSP> objects, ExperimentSP exp) 
  : objects{objects}, exp{exp}
{ }

void Sim::addObjectToSim(ComponentSP c)
{
  for(auto eqtn: c->element->eqtns)
  {
    auto cpy = eqtn->clone();
    setToZero(cpy);
    psys.insert({c, cpy});
  }
}

//TODO: This should be a semantic action?
VarRefSP getControlled(ConnectableSP c)
{
  if(c->neighbor != nullptr) return getControlled(c->neighbor);

  if(c->kind() != Connectable::Kind::SubComponent)
    throw runtime_error{"well fuck"};

  auto x = static_pointer_cast<SubComponentRef>(c);
  return make_shared<VarRef>(x->component, x->subname->value);
}

void Sim::liftControlledSimVars(SubComponentRefSP c)
{
  VarRefSP under_control = getControlled(c);

  for(auto eqtn_p: psys)
  {
    if(eqtn_p.first->name == under_control->component->name)
      liftControlledVars(eqtn_p.second, under_control->name);
  }
}

void Sim::buildSystemEquations()
{
  for(auto c: exp->components)
  {
    if(c->element->kind() == Decl::Kind::Object) addObjectToSim(c);
  }


  for(ConnectionSP cx : exp->connections)
  {
    if(isa(cx->from, Connectable::Kind::SubComponent))
    {
      auto sc = static_pointer_cast<SubComponentRef>(cx->from);
      if(isa(sc->component->element, Decl::Kind::Controller))
      {
        liftControlledSimVars(sc); 
      }
    }
  }

  addCVarResiduals();
}

void Sim::buildPhysics()
{
  for(ComponentSP c : exp->components) c->applyParameters();
  buildSymbolSet();
  buildInitials();
  buildSystemEquations();
}

SimEx Sim::buildSimEx(size_t N)
{
  SimEx sx{psys.size()}; 
  sx.computeNodes = buildComputeTopology(N);
  for(ComputeNode &c: sx.computeNodes) 
    sx.computeNodeSources.push_back(c.emitSource());

  return sx;
}

vector<RVar> Sim::mapVariables(vector<ComputeNode> &topo)
{
  size_t N = topo.size();
  vector<RVar> m;

  EqtnVarCollector evc;
  for(ComponentSP cp: exp->components) 
  {
    if(cp->element->kind() != Decl::Kind::Object) continue;
    evc.run(cp);
  }

  if(N > evc.vars.size())
    throw runtime_error(
        "The number of compute nodes is greater than" 
        "the number of system variables");

  size_t L = ceil(static_cast<double>(evc.vars.size()) / N);

  size_t i{0};
  for(auto v: evc.vars)
  {
    m.push_back( RVar{v, DCoordinate{i/L, i, i%L}} );
    ++i;
  }
  
  for(RVar v: m)
  {
    if(v.coord.px >= N) throw runtime_error("Variable Balderdashery!");
    topo[v.coord.px].vars.push_back(v.var);
    topo[v.coord.px].initials[v.coord.lx].v = initial_state[v.var];
    topo[v.coord.px].initials[v.coord.lx].d = initial_trajectory[v.var];
  }

  return m;
}

vector<REqtn> Sim::mapEquations(vector<ComputeNode> &topo)
{
  vector<REqtn> m;
  size_t N = topo.size();

  size_t L = ceil(static_cast<double>(psys.size()) / N);

  size_t i{0};
  for(auto eqtn_p: psys)
  {
    m.push_back( REqtn{eqtn_p.second->clone(), eqtn_p.first, DCoordinate{i/L, i, i%L}} );
    ++i;
  }
  
  for(REqtn e: m)
  {
    if(e.coord.px >= N) 
      throw runtime_error("Equation Balderdashery! " + to_string(e.coord.px));
    topo[e.coord.px].eqtns.insert({e.component, e.eqtn});
  }

  return m;
}

void addRVars(ComputeNode &n, vector<RVar> &rvars)
{
  EqtnVarCollector evc;
  for(auto p: n.eqtns) evc.run(p.first);

  for(VarRefSP v: evc.vars)
  {
    if(v->kind() == VarRef::Kind::Derivative) continue;
    auto lit =
      find_if(n.vars.begin(), n.vars.end(),
          [v](VarRefSP x){ return x->qname() == v->qname(); });

    if(lit != n.vars.end()) continue;

    auto rit =
      find_if(rvars.begin(), rvars.end(),
        [v](RVar r){ return r.var->qname() == v->qname(); }); //dumb! need eq op!

    if(rit == rvars.end()) throw runtime_error("Hocus Pocus! " + v->qname());

    n.rvars.push_back(*rit);
  }
}

void Sim::addCVarResiduals()
{
  auto cvx = VarExtractorFactory::CVarExtractor();
  for(auto eqtn_p: psys) cvx.run(eqtn_p.first, eqtn_p.second);

  controlled_vars.insert(cvx.vars.begin(), cvx.vars.end());

  for(auto p: cvx.vars)
  {
    //-1 indicates generated code e.g., there is no source line
    static constexpr int nosrc{-1};
    EquationSP eq = make_shared<Equation>(nosrc, nosrc); 
    eq->lhs = make_shared<Real>(0, nosrc, nosrc);
    eq->rhs = 
      make_shared<Subtract>(
        make_shared<CVar>(make_shared<Symbol>(p->name, nosrc, nosrc)),
        make_shared<CCVar>(make_shared<Symbol>(p->name, nosrc, nosrc)),
        nosrc, nosrc);
    psys.insert({p->component, eq});
  }
}

void Sim::buildSymbolSet()
{
  auto ext = VarExtractorFactory::AnyVarExtractor();  
  for(ComponentSP c : exp->components) 
  {
    if(c->element->kind() == Decl::Kind::Object)
      ext.run(c);  
  }
  vars.insert(ext.vars.begin(), ext.vars.end());
}

void Sim::buildInitials()
{
  for(ComponentSP c : exp->components)
  {
    if(c->element->kind() != Decl::Kind::Object) continue;

    for(auto p: c->initials)
    {
      if(p.first->kind() == VarRef::Kind::Normal)
        initial_state[p.first] = p.second->value;

      else if(p.first->kind() == VarRef::Kind::Derivative)
        initial_trajectory[p.first] = p.second->value;
    }
  }
}

vector<ComputeNode> Sim::buildComputeTopology(size_t N)
{
  vector<ComputeNode> topo(N);

  vector<RVar> vars = mapVariables(topo);
  vector<REqtn> eqtns = mapEquations(topo);

  size_t i{0};
  for(ComputeNode &n: topo)
  {
    n.N = vars.size();
    n.expInfo = exp->name->value;
    addRVars(n, vars);
    n.id = i++;
    std::cout << n << std::endl;
  }

  return topo;
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

//Cypress::EqtnVarCollector ---------------------------------------------------

void EqtnVarCollector::run(ComponentSP c)
{
  component = c;
  for(EquationSP eqtn : c->element->eqtns) eqtn->accept(*this);
}

void EqtnVarCollector::in(SymbolSP s)
{
  if(in_derivative)
    vars.insert(make_shared<DVarRef>(component, s->value, derivative_order));
  else
    vars.insert(make_shared<VarRef>(component, s->value));
}

void EqtnVarCollector::visit(DifferentiateSP)
{
  in_derivative = true;
  ++derivative_order;
}

void EqtnVarCollector::leave(DifferentiateSP)
{
  in_derivative = false;
  --derivative_order;
}
  
void EqtnVarCollector::visit(CVarSP)
{
  in_cvar = true;
}

void EqtnVarCollector::leave(CVarSP)
{
  in_cvar = false;
}
