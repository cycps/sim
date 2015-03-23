#include "Cypress/Compiler/Sema.hxx"
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include <set>

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

using namespace cypress;
using namespace cypress::compile;
using std::find_if;
using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;
using std::make_shared;
using std::runtime_error;
using std::stringstream;
using std::shared_ptr;
using std::set_difference;
using std::pair;
using std::transform;
using std::back_inserter;
using std::inserter;
using std::set;
using std::find_if;
using std::static_pointer_cast;

Sema::Sema(SimulationSP sim, DiagnosticReportSP dr, vector<ObjectSP> objects)
  : sim{sim}, dr{dr}, objects{objects}
{}

void Sema::check()
{
  for(ObjectSP o : objects) check(o);
  for(ComponentSP c : sim->components) check(c);
  for(ConnectionSP c : sim->connections) check(c);

  if(dr->catastrophic()) throw CompilationError(*dr);
}


void Sema::check(ComponentSP c)
{
  typeCheck(c);
  //if type checking fails cannot conduct further checks
  if(dr->catastrophic()) throw CompilationError(*dr);

  paramsCheck(c);
}

double Sema::tryGetFloatingParam(ComponentSP c, string pname)
{
  double value;
  try { value = stod(c->parameterValue(pname)); }
  catch(std::invalid_argument &)
  {
    SymbolSP sym = c->parameter(pname);
    dr->diagnostics.push_back({
        Diagnostic::Level::Error,
        "Invalid actuator "+pname+" value, floating point number is required",
        sym->line, sym->column});
  }
  catch(std::out_of_range &)
  {
    SymbolSP sym = c->parameter(pname);
    dr->diagnostics.push_back({
        Diagnostic::Level::Error,
        "Invalid actuator "+pname+" value, argument out of range",
        sym->line, sym->column});
  }
  catch(ParameterNotFound &)
  {
    dr->diagnostics.push_back({
        Diagnostic::Level::Error,
        "Actuator instantiation requires a "+pname+" argument",
        c->name->line, c->name->column});
  }
  
  return value;
}

void Sema::typeCheck(ComponentSP c)
{
  if(c->kind->value == "Actuator")
  {
    c->element = make_shared<Actuator>(c->name, c->kind->line, c->kind->column);
    auto attrs = make_shared<ActuatorAttributes>();
    c->attributes = attrs;

    attrs->min = tryGetFloatingParam(c, "Min");
    attrs->max = tryGetFloatingParam(c, "Max");
    attrs->dmin = tryGetFloatingParam(c, "DMin");
    attrs->dmax = tryGetFloatingParam(c, "DMax");

    return;
  }
  else if(c->kind->value == "Sensor")
  {
    c->element = make_shared<Sensor>(c->name, c->kind->line, c->kind->column);
    c->attributes = make_shared<SensorAttributes>();
    return;
  }

  for(auto obj : objects)
  {
    if(c->kind->value == obj->name->value)
    {
      c->element = obj;
      return;
    }
  }

  dr->diagnostics.push_back({
      Diagnostic::Level::Error,
      "Undefined Component Type: " + c->kind->value,
      c->kind->line, c->kind->column
      });
}

void Sema::paramsCheck(ComponentSP c)
{
  set<string> supplied;
  transform(c->params.begin(), c->params.end(), 
      inserter(supplied, supplied.begin()),
      [](pair<SymbolSP, string> x){ return x.first->value; });

  set<string> required;
  transform(c->element->params.begin(), c->element->params.end(), 
      inserter(required, required.begin()),
      [](SymbolSP x){ return x->value; });

  vector<string> required_but_not_supplied;

  set_difference(
      required.begin(), required.end(),
      supplied.begin(), supplied.end(),
      back_inserter(required_but_not_supplied));

  if(!required_but_not_supplied.empty())
  {
    string missing_params{""};
    for(size_t i=0; i<required_but_not_supplied.size()-1; ++i)
      missing_params += required_but_not_supplied[i] + ", ";
    missing_params += required_but_not_supplied.back();

    string diag_string = 
      "The element " + c->element->name->value + " requires the parameters {" +
      missing_params + "}";

    dr->diagnostics.push_back({
        Diagnostic::Level::Error, diag_string, c->name->line, c->name->column});
  }

  vector<string> supplied_but_not_required;
  set_difference(
      supplied.begin(), supplied.end(),
      required.begin(), required.end(),
      back_inserter(supplied_but_not_required));

  if(!supplied_but_not_required.empty())
  {
    string extra_params{""};
    for(size_t i=0; i<supplied_but_not_required.size()-1; ++i)
      extra_params += supplied_but_not_required[i] + ", ";
    extra_params += supplied_but_not_required.back();

    string diag_string = 
      "The element " + c->element->name->value 
      + " does not take the parameters {" 
      + extra_params + "} they will be ignored";
    
    dr->diagnostics.push_back({
        Diagnostic::Level::Warning, diag_string, 
        c->name->line, c->name->column});
  }
}

void Sema::check(ObjectSP)
{
  //inputCheck(o);
}

/*
void Sema::inputCheck(ObjectSP o)
{
  for(SymbolSP in: o->inputs)
  {
    VarLifter<CVar> vl(in->value);  
    for(EquationSP eq: o->eqtns) eq->accept(vl);

    if(vl.lift_count == 0)
      dr->diagnostics.push_back({
          Diagnostic::Level::Warning,
          "The input " + in->value 
          + " does not apply to any equation variables"
          + " for the object " + o->name->value,
          in->line, in->column});
  }

  //TODO: repeated variable

}
*/

void Sema::check(ConnectionSP c)
{
  //TODO since all connections are between subcomponents now we can
  //get rid of this nonsense entirely at some point
  if(!isa(c->from, Connectable::Kind::SubComponent) ||
     !isa(c->to, Connectable::Kind::SubComponent))
  {
    dr->diagnostics.push_back({
        Diagnostic::Level::Error,
        "Illegal connection, connections must be between subcomponents",
        c->from->name->line, c->from->name->column});
    throw CompilationError{*dr};
  }

  SubComponentRefSP from = static_pointer_cast<SubComponentRef>(c->from),
                    to = static_pointer_cast<SubComponentRef>(c->to);
  
  check(from);
  check(to);
  //The above checks establish the components so we cannot continue if
  //this part of the data structure is not there
  if(dr->catastrophic()) throw CompilationError{*dr};
 
  //Checking the nature of the connections themselves
  //Sensor Checks
  if(from->component->element->kind() == Element::Kind::Sensor)
  {
    if(to->component->element->kind() != Element::Kind::Object)
      dr->diagnostics.push_back({
          Diagnostic::Level::Error,
          "Illegal sensor connection, sensors can only sense physical objects",
          c->to->name->line, c->to->name->column});

    static_pointer_cast<SensorAttributes>(from->component->attributes)->target = 
      make_shared<VarRef>(to->component, to->subname->value);
  }
    
  if(to->component->element->kind() == Element::Kind::Sensor)
  {
    if(from->component->element->kind() != Element::Kind::Object)
      dr->diagnostics.push_back({
          Diagnostic::Level::Error,
          "Illegal sensor connection, sensors can only sense physical objects",
          c->from->name->line, c->from->name->column});

    auto attrs = 
      static_pointer_cast<SensorAttributes>(to->component->attributes);

    attrs->target = make_shared<VarRef>(from->component, from->subname->value);

    try
    {
      attrs->rate = stoul(to->component->parameterValue("Rate"));
    }
    catch(std::invalid_argument &)
    {
      SymbolSP sym = to->component->parameter("Rate");
      dr->diagnostics.push_back({
          Diagnostic::Level::Error,
          "Invalid sensor rate, positive integral number is required",
          sym->line, sym->column});
    }
    catch(std::out_of_range &)
    {
      SymbolSP sym = to->component->parameter("Rate");
      dr->diagnostics.push_back({
          Diagnostic::Level::Error,
          "Invalid sensor rate, argument out of range",
          sym->line, sym->column});
    }
    catch(ParameterNotFound &)
    {
      dr->diagnostics.push_back({
          Diagnostic::Level::Error,
          "Sensor instantiation requires a Rate argument",
          to->component->name->line, to->component->name->column});
    }
    try
    {
      attrs->destination = to->component->parameterValue("Destination");

      struct sockaddr_in servaddr;
      bzero(&servaddr, sizeof(servaddr));
      servaddr.sin_family = AF_INET;
      servaddr.sin_port = htons(4747);
      int err = 
        inet_pton(AF_INET, attrs->destination.c_str(), &servaddr.sin_addr);

      if(err < 0)
      {
        SymbolSP sym = to->component->parameter("Destination");
        dr->diagnostics.push_back({
            Diagnostic::Level::Error,
            "Invalid destination, must be a valid IP address or dns name",
            sym->line, sym->column});
      }

    }
    catch(ParameterNotFound &)
    {
      dr->diagnostics.push_back({
          Diagnostic::Level::Error,
          "Sensor instantiate requires a Destination argument",
          to->component->name->line, to->component->name->column});
    }
  }
  
  //Actuator Checks
  if(from->component->element->kind() == Element::Kind::Actuator)
  {
    if(to->component->element->kind() != Element::Kind::Object)
      dr->diagnostics.push_back({
          Diagnostic::Level::Error,
          "Illegal actuator connection, sensors can only sense physical objects",
          c->to->name->line, c->to->name->column});

    auto attrs = 
      static_pointer_cast<ActuatorAttributes>(from->component->attributes);

    attrs->target = make_shared<VarRef>(to->component, to->subname->value);
  }
    
  if(to->component->element->kind() == Element::Kind::Actuator)
  {
    if(from->component->element->kind() != Element::Kind::Object)
      dr->diagnostics.push_back({
          Diagnostic::Level::Error,
          "Illegal actuator connection, sensors can only sense physical objects",
          c->from->name->line, c->from->name->column});
    
    auto attrs = 
      static_pointer_cast<ActuatorAttributes>(to->component->attributes);
    
    attrs->target = make_shared<VarRef>(from->component, from->subname->value);
  }

    
  /*
  if(c->from->kind() == Connectable::Kind::Component || 
     c->from->kind() == Connectable::Kind::SubComponent)
  {
    check(static_pointer_cast<ComponentRef>(c->from));
  }
  if(c->to->kind() == Connectable::Kind::Component || 
     c->to->kind() == Connectable::Kind::SubComponent)
  {
    check(static_pointer_cast<ComponentRef>(c->to));
  }
  */
}

void Sema::check(SubComponentRefSP c)
{
  
  auto &cs = sim->components;
  auto ref = find_if(cs.begin(), cs.end(),
      [c](ComponentSP x){ return x->name->value == c->name->value; });

  if(ref == cs.end())
  {
    dr->diagnostics.push_back({
        Diagnostic::Level::Error,
        "Undefined component reference `" + c->name->value + "`",
        c->name->line, c->name->column});
  
    return;
  }

  c->component = *ref;

  string sr = static_pointer_cast<SubComponentRef>(c)->subname->value;
  VarCollector vc;
  vc.run(c->component->element);
  auto vars = vc.vars[c->component->element];

  auto subref = find_if(vars.begin(), vars.end(),
      [sr](SymbolSP x){ return x->value == sr; });

  if(subref == vars.end())
  {
    dr->diagnostics.push_back({
        Diagnostic::Level::Error,
        "Undefined component subreference `" + c->name->value +"."+ sr + "`",
        c->name->line, c->name->column});
  }
}

//Var Collector ===============================================================
void VarCollector::run(ElementSP e)
{
  elem = e;
  for(EquationSP eqtn : elem->eqtns) eqtn->accept(*this);
}

void VarCollector::visit(SymbolSP s)
{
   if(dblock) return; 
   if(find_if(elem->params.begin(), elem->params.end(), 
         [s](SymbolSP x){ return s->value == x->value; })
         != elem->params.end())
     return;

   vars[elem].insert(s); 
}

void VarCollector::leave(DifferentiateSP)
{
  dblock = false;
}

void VarCollector::visit(DifferentiateSP s)
{
  //TODO dupcheck?
  derivs[elem].insert(s->arg);
  dblock = true;
}

void VarCollector::showVars()
{
  cout << "vars:" << endl;
  for(auto p : vars)
    for(auto v : p.second)
      cout << p.first->name->value << "." << v->value << endl;
  cout << endl;
}

void VarCollector::showDerivs()
{
  cout << "derivs:" << endl;
  for(auto p : derivs)
    for(auto d : p.second)
      cout << p.first->name->value << "." << d->value << endl;
  cout << endl;
}



// Semantic Checks ============================================================

DiagnosticReport 
cypress::compile::check(ExperimentSP ex, vector<ElementSP> &elements)
{
  DiagnosticReport diags;

  for(ComponentSP c : ex->components)
  {
    check(c, elements, diags);
  }

  for(ConnectionSP c : ex->connections)
  {
    checkConnection(c, ex->components, diags);
  }

  return diags;
}

DiagnosticReport&
cypress::compile::check(ComponentSP c, vector<ElementSP> &elements, 
    DiagnosticReport &dr)
{
  checkComponentType(c, elements, dr);
  if(dr.catastrophic())
    throw CompilationError{dr};

  /*
  liftComponentInputs(c, elements, dr);
  if(dr.catastrophic())
    throw CompilationError{dr};
    */

  checkComponentParams(c, dr);
  
  return dr;
}

DiagnosticReport&
cypress::compile::checkComponentType(ComponentSP c, 
    std::vector<ElementSP> &elements, DiagnosticReport &dr)
{
  //TODO: out of scope?
  if(c->kind->value == "Link") 
  {
    c->element = make_shared<Link>(c->name, c->kind->line, c->kind->column);
    return dr;
  }

  if(c->kind->value == "Actuator")
  {
    c->element = make_shared<Actuator>(c->name, c->kind->line, c->kind->column);
    return dr;
  }

  for(auto e : elements)
  {
    if(c->kind->value == e->name->value)
    {
      c->element = e;
      return dr;
    }
  }

  dr.diagnostics.push_back({
      Diagnostic::Level::Error,
      "Undefined Component Type: " + c->kind->value,
      c->kind->line, c->kind->column
      });

  return dr;
}

DiagnosticReport&
cypress::compile::checkComponentParams(ComponentSP c, DiagnosticReport &dr)
{
  set<string> supplied;
  transform(c->params.begin(), c->params.end(), 
      inserter(supplied, supplied.begin()),
      [](pair<SymbolSP, string> x){ return x.first->value; });

  set<string> required;
  transform(c->element->params.begin(), c->element->params.end(), 
      inserter(required, required.begin()),
      [](SymbolSP x){ return x->value; });

  vector<string> required_but_not_supplied;

  set_difference(
      required.begin(), required.end(),
      supplied.begin(), supplied.end(),
      back_inserter(required_but_not_supplied));

  if(!required_but_not_supplied.empty())
  {
    string missing_params{""};
    for(size_t i=0; i<required_but_not_supplied.size()-1; ++i)
      missing_params += required_but_not_supplied[i] + ", ";
    missing_params += required_but_not_supplied.back();

    string diag_string = 
      "The element " + c->element->name->value + " requires the parameters {" +
      missing_params + "}";

    dr.diagnostics.push_back({
        Diagnostic::Level::Error, diag_string, c->name->line, c->name->column});

  }

  vector<string> supplied_but_not_required;
  set_difference(
      supplied.begin(), supplied.end(),
      required.begin(), required.end(),
      back_inserter(supplied_but_not_required));

  if(!supplied_but_not_required.empty())
  {
    string extra_params{""};
    for(size_t i=0; i<supplied_but_not_required.size()-1; ++i)
      extra_params += supplied_but_not_required[i] + ", ";
    extra_params += supplied_but_not_required.back();

    string diag_string = 
      "The element " + c->element->name->value + " does not take the parameters {" +
      extra_params + "} they will be ignored";
    
    dr.diagnostics.push_back({
        Diagnostic::Level::Warning, diag_string, c->name->line, c->name->column});
  }

  return dr;
}

DiagnosticReport&
cypress::compile::checkConnection(ConnectionSP c, vector<ComponentSP> &cs, 
    DiagnosticReport &dr)
{
  if(c->from->kind() == Connectable::Kind::Component || 
     c->from->kind() == Connectable::Kind::SubComponent)
  {
    checkComponentRef(static_pointer_cast<ComponentRef>(c->from), cs, dr);
  }
  if(c->to->kind() == Connectable::Kind::Component || 
     c->to->kind() == Connectable::Kind::SubComponent)
  {
    checkComponentRef(static_pointer_cast<ComponentRef>(c->to), cs, dr);
  }
  return dr;
}

DiagnosticReport&
cypress::compile::checkComponentRef(ComponentRefSP c, vector<ComponentSP> &cs,
    DiagnosticReport &dr)
{
  auto ref = find_if(cs.begin(), cs.end(),
      [c](ComponentSP x){ return x->name->value == c->name->value; });
 
  if(ref == cs.end())
  {
    dr.diagnostics.push_back({
        Diagnostic::Level::Error,
        "Undefined component reference `" + c->name->value + "`",
        c->name->line, c->name->column});
    return dr;
  }

  c->component = *ref;

  if(c->kind() == Connectable::Kind::SubComponent)
  {
    string sr = static_pointer_cast<SubComponentRef>(c)->subname->value;
    VarCollector vc;
    vc.run(c->component->element);
    auto vars = vc.vars[c->component->element];

    auto subref = find_if(vars.begin(), vars.end(),
        [sr](SymbolSP x){ return x->value == sr; });

    if(subref == vars.end())
    {
      dr.diagnostics.push_back({
          Diagnostic::Level::Error,
          "Undefined component subreference `" + c->name->value +"."+ sr + "`",
          c->name->line, c->name->column});
    }
  }

  return dr;
}
