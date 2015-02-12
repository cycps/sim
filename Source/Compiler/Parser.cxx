#include "Cypress/Compiler/Parser.hxx"
#include "Cypress/Compiler/DeclRegex.hxx"
#include <iostream>
#include <stdexcept>
#include <regex>
#include <string>
#include <vector>
#include <sstream>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-register"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp> 
#pragma clang diagnostic pop
#include <boost/lexical_cast.hpp>

using namespace cypress;
using namespace cypress::compile;

using std::string;
using std::cout;
using std::endl;
using std::runtime_error;
using std::invalid_argument;
using std::regex;
using std::regex_match;
using std::to_string;
using std::smatch;
using std::shared_ptr;
using std::make_shared;
using std::vector;
using std::copy_if;
using std::back_inserter;
using std::for_each;
using std::remove_if;
using std::transform;
using std::stringstream;
using std::getline;
using std::unordered_map;

Parser::Parser(string source)
  : source{source}
{}

shared_ptr<Decls> Parser::run()
{
  split(source, '\n', lines);
  auto decls = make_shared<Decls>();

  for(size_t i=0; i<lines.size(); ++i)
  {
    currline = i;
    DeclType dt;
    LineType lt = classifyLine(lines[i], dt);
    switch(lt)
    {
      case LineType::Decl : i += parseDecl(i, dt, decls); break;
      case LineType::Code : 
        throw runtime_error("[" + to_string(i+1) + "] orphan code");
      case LineType::Comment : break;
      case LineType::Empty : break;
      case LineType::SomethingElse : 
        throw runtime_error(
            "[" + to_string(i+1) + "] not sure what this line is doing");
    }
  }

  //cout << *decls;
  return decls;
}

size_t Parser::parseDecl(size_t at, DeclType dt, shared_ptr<Decls> decls)
{
  size_t advance{0};
  switch(dt)
  {
    case DeclType::Object : 
    {
      auto obj = parseObject(at, advance);
      decls->objects.push_back(obj);
      return advance;
    }

    case DeclType::Controller: 
    {
      auto controller = parseController(at, advance);
      decls->controllers.push_back(controller);
      return advance;
    }

    case DeclType::Experiment: 
    {
      auto experiment = parseExperiment(at, advance);
      decls->experiments.push_back(experiment);
      return advance;
    }
  }
}
    
LineType Parser::classifyLine(const string &s, DeclType &dt)
{
  if(isDecl(s, dt)) return LineType::Decl;
  if(isCode(s)) return LineType::Code;
  if(isComment(s)) return LineType::Comment;
  if(isEmpty(s)) return LineType::Empty;
  return LineType::SomethingElse;
}

ObjectSP Parser::parseObject(size_t at, size_t &lc)
{
  smatch sm;
  regex_match(lines[at], sm, objrx());
  auto object = make_shared<Object>(make_shared<Symbol>(sm[1], at));

  //Parse the parameters
  string _pstr = sm[2];
  string pstr = string(_pstr.begin()+1, _pstr.end()-1);
  auto params = split(pstr, ',');

  transform(params.begin(), params.end(), back_inserter(object->params),
      [at](const string &ps){ return make_shared<Symbol>(ps, at); });

  size_t idx = at+1;
  currline = idx;
  while(isCode(lines[idx]) || isEmpty(lines[idx]))
  { 
    if(isEmpty(lines[idx])) {}
    else if(isComment(lines[idx])) {}
    else 
    {
      if(isEqtn(lines[idx])) 
      {
        EquationSP eqtn = parseEqtn(lines[idx]);
        object->eqtns.push_back(eqtn);
      }
    }
    ++idx; 
    if(idx >= lines.size()) break;
    currline = idx;
  }
  lc = idx - at - 1;
  return object;
}
    
ControllerSP Parser::parseController(size_t at, size_t &lc)
{
  smatch sm;
  regex_match(lines[at], sm, contrx());
  auto controller = make_shared<Controller>(make_shared<Symbol>(sm[1], currline));

  string _pstr = sm[2];
  string pstr = string(_pstr.begin()+1, _pstr.end()-1);
  auto params = split(pstr, ',');

  transform(params.begin(), params.end(), back_inserter(controller->params),
      [this](const string &ps){ return make_shared<Symbol>(ps, currline); });

  size_t idx = at+1;
  currline = idx;
  while(isCode(lines[idx]) || isEmpty(lines[idx]))
  { 
    if(isEmpty(lines[idx])) {}
    else if(isComment(lines[idx])) {}
    else 
    {
      if(isEqtn(lines[idx])) 
      {
        EquationSP eqtn = parseEqtn(lines[idx]);
        controller->eqtns.push_back(eqtn);
      }
    }
    ++idx; 
    if(idx >= lines.size()) break;
    currline = idx;
  }
  lc = idx - at - 1;
  return controller;
}

ExperimentSP Parser::parseExperiment(size_t at, size_t &lc)
{
  smatch sm;
  regex_match(lines[at], sm, exprx());
  auto experiment = make_shared<Experiment>(make_shared<Symbol>(sm[1], currline));
  size_t idx = at+1;
  currline = idx;

  while(isCode(lines[idx]) || isEmpty(lines[idx]))
  { 
    if(isEmpty(lines[idx])) {}
    else if(isComment(lines[idx])) {}
    else 
    {
      if(regex_match(lines[idx], sm, comprx())) 
      {
        ComponentSP cp = parseComponent(lines[idx]);
        experiment->components.push_back(cp);
      }
      else if(regex_match(lines[idx], sm, lnkrx()))
      {
        vector<ConnectionSP> cnx = parseConnectionStmt(lines[idx]);
        experiment->connections.insert(
            experiment->connections.end(), cnx.begin(), cnx.end());
      }
    }
    ++idx; 
    if(idx >= lines.size()) break;
    currline = idx;
  }
  lc = idx - at - 1;
  return experiment;
}


vector<ConnectionSP> Parser::parseConnectionStmt(const string &s)
{
  auto links = split(s, '>');
  for(string &l : links)
    l.erase(remove_if(l.begin(), l.end(), isspace), l.end());

  unordered_map<string, ConnectableSP> table;
  vector<ConnectionSP> lnks;
  smatch sm;
  for(size_t i=0; i<links.size()-1; ++i)
  {
    ConnectableSP from, to;
    string from_name, to_name;

    if(regex_match(links[i], sm, thingrx()))
    {
      from_name = sm[1];
      if(table.find(from_name) == table.end())
      {
        table[from_name] = 
          make_shared<ComponentRef>(make_shared<Symbol>(sm[1], currline));
      }
      from = table[from_name];
    }
    else if(regex_match(links[i], sm, subthingrx()))
    {
      from_name = string(sm[1])+"."+string(sm[2]);
      if(table.find(from_name) == table.end())
      {
        table[from_name] = 
          make_shared<SubComponentRef>(
            make_shared<Symbol>(sm[1], currline),
            make_shared<Symbol>(sm[2], currline));
      }
      from = table[from_name];
    }
    else if(regex_match(links[i], sm, atodrx()))
    {
      auto uuid = boost::uuids::random_generator()();
      from_name = "AtoD::"+boost::lexical_cast<string>(uuid);
      from = make_shared<AtoD>(stod(sm[1]));
    }
    else
      throw runtime_error{"disformed linkable : " + links[i]};

    
    if(regex_match(links[i+1], sm, thingrx()))
    {
      to_name = sm[1];
      if(table.find(to_name) == table.end())
      {
        table[to_name] = 
          make_shared<ComponentRef>(make_shared<Symbol>(sm[1], currline));
      }
      to = table[to_name];
    }
    else if(regex_match(links[i+1], sm, subthingrx()))
    {
      to_name = string(sm[1])+"."+string(sm[2]);
      if(table.find(to_name) == table.end())
      {
        table[to_name] =
          make_shared<SubComponentRef>(
              make_shared<Symbol>(sm[1], currline),
              make_shared<Symbol>(sm[2], currline));
      }
      to = table[to_name];
    }
    else if(regex_match(links[i+1], sm, atodrx()))
    {
      auto uuid = boost::uuids::random_generator()();
      to_name = "AtoD::"+boost::lexical_cast<string>(uuid);
      to = make_shared<AtoD>(stod(sm[1]));
    }
    else
      throw runtime_error{"disformed linkable" + links[i+1]};

    //from->neighbors.push_back(to);
    from->neighbor = to;
    //Directed neighbors only for the time being
    //to->neighbors.push_back(from);
    lnks.push_back(make_shared<Connection>(from, to));
  }

  return lnks;
}

bool Parser::isDecl(const string &s, DeclType &dt)
{
  string Object{"Object"}, 
         Controller{"Controller"},
         Experiment{"Experiment"};

  smatch sm;

  if(regex_match(s, sm, objrx())) { dt = DeclType::Object; return true; }
  if(regex_match(s, sm, contrx())) { dt = DeclType::Controller; return true; }
  if(regex_match(s, sm, exprx())) { dt = DeclType::Experiment; return true; }
  
  return false;
}

bool Parser::isCode(const string &s)
{
  regex rx{"\\s\\s+\\S+.*"};
  return regex_match(s, rx);
}

bool Parser::isComment(const string &s)
{
  regex rx{"\\s*//.*"};
  return regex_match(s, rx);
}

bool Parser::isEmpty(const string &s)
{
  regex rx{"\\s*"};
  return regex_match(s, rx);
}
    
bool Parser::isEqtn(const string &s)
{
  regex rx{".*=.*"};
  return regex_match(s, rx);
}

EquationSP Parser::parseEqtn(const string &s)
{
  regex rx{"(.*)=(.*)"};
  smatch sm;
  regex_match(s, sm, rx);
  
  auto eqtn = make_shared<Equation>(currline);
  if(sm.size() != 3) throw runtime_error("disformed equation");
  string lhs = sm[1],
         rhs = sm[2];

  //remove whitespace
  lhs.erase(remove_if(lhs.begin(), lhs.end(), isspace), lhs.end());
  rhs.erase(remove_if(rhs.begin(), rhs.end(), isspace), rhs.end());

  eqtn->lhs = parseExpr(lhs);
  eqtn->rhs = parseExpr(rhs);

  return eqtn;
}
    
ExpressionSP Parser::parseExpr(const string &s)
{
  regex rx{"([a-zA-Zα-ωΑ-Ω0-9'/\\*\\^ ]+)(?:([\\+\\-])(.*))?"};
  smatch sm;
  regex_match(s, sm, rx);
  vector<string> matches;
  TermSP lhs{nullptr};
  ExpressionSP rhs{nullptr};
  if(sm.size()>1)
  {
    copy_if(sm.begin()+1, sm.end(), back_inserter(matches), 
        [](const string &m){ return !m.empty(); });
    switch(matches.size())
    {
      //Unary Term
      case 1: return parseTerm(matches[0]); 
      //Binary Term
      case 3: lhs = parseTerm(matches[0]);
              rhs = parseExpr(matches[2]);
              switch(matches[1][0])
              {
                case '+' : return make_shared<Add>(lhs, rhs, currline);
                case '-' : return make_shared<Subtract>(lhs, rhs, currline);
              }
    }
  }
  return nullptr;
}
    
TermSP Parser::parseTerm(const string &s)
{
  regex rx{"([a-zA-Zα-ωΑ-Ω0-9'\\^ ]+)(?:([\\*/])(.*))?"};
  smatch sm;
  regex_match(s, sm, rx);
  vector<string> matches;
  FactorSP lhs{nullptr};
  TermSP rhs{nullptr};
  if(sm.size()>1)
  {
    copy_if(sm.begin()+1, sm.end(), back_inserter(matches), 
        [](const string &m){ return !m.empty(); });
    switch(matches.size())
    {
      case 1: return parseFactor(matches[0]);
      case 3: lhs = parseFactor(matches[0]);
              rhs = parseTerm(matches[2]);
              switch(matches[1][0])
              {
                case '*' : return make_shared<Multiply>(lhs, rhs, currline);
                case '/' : return make_shared<Divide>(lhs, rhs, currline);
              }
    }
  }
  return nullptr;
}
    
FactorSP Parser::parseFactor(const string &s)
{
  regex rx{"([a-zA-Zα-ωΑ-Ω0-9]+)(?:(['\\^])(.*))?"};
  smatch sm;
  regex_match(s, sm, rx);
  vector<string> matches;
  FactorSP lhs{nullptr};
  TermSP rhs{nullptr};
  if(sm.size()>1)
  {
    copy_if(sm.begin()+1, sm.end(), back_inserter(matches),
        [](const string &m){ return !m.empty(); });
    if(matches.size()==1)
      return parseAtom(matches[0]);

    else if(matches.size()==2 && matches[1][0] == '\'')
      return parseDerivative(matches[0]);

    else if(matches.size()==3 && matches[1][0] == '^')
      return parsePow(matches[0], matches[2]);

    throw runtime_error("disformed factor");
  }

  return nullptr;
}
    
AtomSP Parser::parseAtom(const string &s)
{
  double value{0}; 
  try
  { 
    value = stod(s); 
    return make_shared<Real>(value, currline);
  }
  catch(const invalid_argument &e)
  {
    return make_shared<Symbol>(s, currline);
  }

  return nullptr;
}
    
DifferentiateSP Parser::parseDerivative(const string &s)
{
  return make_shared<Differentiate>(make_shared<Symbol>(s, currline), currline);
}
    
PowSP Parser::parsePow(const string &lower, const string &upper)
{
  return make_shared<Pow>(
      make_shared<Symbol>(lower, currline), parseAtom(upper), currline);
}

ComponentSP Parser::parseComponent(const string &s)
{
  smatch sm;
  regex_match(s, sm, comprx());
  if(sm.size() < 3) throw runtime_error("disformed component instance");
  
  auto cp = make_shared<Component>(
              make_shared<Symbol>(sm[1], currline),
              make_shared<Symbol>(sm[2], currline));

  string _pstr = sm[3];
  string pstr = string(_pstr.begin()+1, _pstr.end()-1);
  pstr.erase(remove_if(pstr.begin(), pstr.end(), isspace), pstr.end());

  auto params = split(pstr, ',');
  for(const string &p : params)
  {
    if(p.find(":") != string::npos)
    {
      auto ps = split(p, ':');
      cp->params[make_shared<Symbol>(ps[0], currline)] = 
        make_shared<Real>(stod(ps[1]), currline);
    }
    else if(p.find("|") != string::npos)
    {
      auto ps = split(p, '|');
      cp->initials[make_shared<Symbol>(ps[0], currline)] = 
        make_shared<Real>(stod(ps[1]), currline);
    }
    else
    {
      //TODO: Need to do this with the diagnostics API
      throw runtime_error("disformed component parameterization");
    }
  }

  return cp;
}

vector<string> &
cypress::compile::split( const string &s, char delim, 
    vector<string> &elems) 
{
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}


vector<string> 
cypress::compile::split(const string &s, char delim) 
{
    vector<string> elems;
    split(s, delim, elems);
    return elems;
}
