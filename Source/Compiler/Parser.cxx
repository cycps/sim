#include "Cypress/Core/Var.hxx"
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

void extractParams(const string &s, vector<string> &ps, vector<size_t> &ss, 
    size_t &sp)
{
  //remove parens
  ++sp;
  string pstr = string(s.begin()+1, s.end()-1);

  regex rx{"\\s*(" CHR "*.*)"};

  ps = split(pstr, ',');
  for(string &s : ps)
  {
    smatch sm;
    regex_match(s, sm, rx);
    sp += sm.position(1); //skip whitespace
    ss.push_back(sp);
    sp += sm[1].str().length();
    ++sp; //comma
    s.erase(remove_if(s.begin(), s.end(), isspace), s.end());
  }
}

ObjectSP Parser::parseObject(size_t at, size_t &lc)
{
  smatch sm;
  regex_match(lines[at], sm, objrx());
  auto sym = make_shared<Symbol>(sm[1], at, sm.position(1));
  auto object = make_shared<Object>(sym, at, sm.position(0));

  //Parse the parameters
  //save the column starting point
  size_t sp = sm.position(2);
  string _pstr = sm[2];

  vector<string> params;
  vector<size_t> param_pos;
  extractParams(sm[2], params, param_pos, sp);
  for(size_t i=0; i<params.size(); ++i)
  {
    object->params.push_back(
        make_shared<Symbol>(params[i], at, param_pos[i]));
  }

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
  auto controller = 
    make_shared<Controller>(
        make_shared<Symbol>(sm[1], currline, sm.position(1)),
        currline, sm.position(0)
        );

  size_t sp = sm.position(2);
  
  vector<string> params;
  vector<size_t> param_pos;
  extractParams(sm[2], params, param_pos, sp);
  for(size_t i=0; i<params.size(); ++i)
  {
    controller->params.push_back(
        make_shared<Symbol>(params[i], at, param_pos[i]));
  }

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
  auto experiment = 
    make_shared<Experiment>(
        make_shared<Symbol>(sm[1], currline, sm.position(1)),
        currline, sm.position(0)
      );

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
          make_shared<ComponentRef>(
              make_shared<Symbol>(sm[1], currline, sm.position(1)));
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
            make_shared<Symbol>(sm[1], currline, sm.position(1)),
            make_shared<Symbol>(sm[2], currline, sm.position(2)));
      }
      from = table[from_name];
    }
    else if(regex_match(links[i], sm, atodrx()))
    {
      auto uuid = boost::uuids::random_generator()();
      from_name = "AtoD::"+boost::lexical_cast<string>(uuid);
      from = make_shared<AtoD>(
          make_shared<Real>(stod(sm[1]), currline, sm.position(1))
        );
    }
    else
      throw runtime_error{"disformed linkable : " + links[i]};

    
    if(regex_match(links[i+1], sm, thingrx()))
    {
      to_name = sm[1];
      if(table.find(to_name) == table.end())
      {
        table[to_name] = 
          make_shared<ComponentRef>(
              make_shared<Symbol>(sm[1], currline, sm.position(1)));
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
              make_shared<Symbol>(sm[1], currline, sm.position(1)),
              make_shared<Symbol>(sm[2], currline, sm.position(2)));
      }
      to = table[to_name];
    }
    else if(regex_match(links[i+1], sm, atodrx()))
    {
      auto uuid = boost::uuids::random_generator()();
      to_name = "AtoD::"+boost::lexical_cast<string>(uuid);
      to = make_shared<AtoD>(
          make_shared<Real>(stod(sm[1]), currline, sm.position(1))
        );
    }
    else
      throw runtime_error{"disformed linkable" + links[i+1]};

    from->neighbor = to;
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
  
  if(sm.size() != 3) throw runtime_error("disformed equation");
  auto eqtn = make_shared<Equation>(currline, sm.position(1));
  string lhs = sm[1],
         rhs = sm[2];

  //remove whitespace
  lhs.erase(remove_if(lhs.begin(), lhs.end(), isspace), lhs.end());
  rhs.erase(remove_if(rhs.begin(), rhs.end(), isspace), rhs.end());

  eqtn->lhs = parseExpr(lhs);
  eqtn->rhs = parseExpr(rhs);

  return eqtn;
}

void nonEmptyMatchesAndPositions(const smatch &sm, vector<string> &matches,
    vector<size_t> &positions)
{
  for(size_t i=1; i<sm.size(); i++)
  {
    if(!sm[i].str().empty())
    {
      matches.push_back(sm[i]);
      positions.push_back(sm.position(i));
    }
  }
}
    
ExpressionSP Parser::parseExpr(const string &s)
{
  regex rx{"([a-zA-Zα-ωΑ-Ω0-9'/\\*\\^ ]+)(?:([\\+\\-])(.*))?"};
  smatch sm;
  regex_match(s, sm, rx);
  vector<string> matches;
  vector<size_t> positions;
  TermSP lhs{nullptr};
  ExpressionSP rhs{nullptr};
  if(sm.size()>1)
  {
    nonEmptyMatchesAndPositions(sm, matches, positions);
    switch(matches.size())
    {
      //Unary Term
      case 1: return parseTerm(matches[0]); 
      //Binary Term
      case 3: lhs = parseTerm(matches[0]);
              rhs = parseExpr(matches[2]);
              switch(matches[1][0])
              {
                case '+' : 
                  return make_shared<Add>(lhs, rhs, currline, positions[1]);
                case '-' : 
                  return make_shared<Subtract>(lhs, rhs, currline, positions[1]);
              }
    }
  }
  throw runtime_error{"Malformed Expression: " + s};
}
    
TermSP Parser::parseTerm(const string &s)
{
  regex rx{"([a-zA-Zα-ωΑ-Ω0-9'\\^ ]+)(?:([\\*/])(.*))?"};
  smatch sm;
  regex_match(s, sm, rx);
  vector<string> matches;
  vector<size_t> positions;

  FactorSP lhs{nullptr};
  TermSP rhs{nullptr};


  if(sm.size()>1)
  {
    nonEmptyMatchesAndPositions(sm, matches, positions);
    switch(matches.size())
    {
      case 1: return parseFactor(matches[0]);
      case 3: lhs = parseFactor(matches[0]);
              rhs = parseTerm(matches[2]);
              switch(matches[1][0])
              {
                case '*' : 
                  return make_shared<Multiply>(lhs, rhs, currline, positions[1]);
                case '/' : 
                  return make_shared<Divide>(lhs, rhs, currline, positions[1]);
              }
    }
  }
  throw runtime_error{"Malformed Term: " + s};
}
    
FactorSP Parser::parseFactor(const string &s)
{
  regex rx{"([a-zA-Zα-ωΑ-Ω0-9]+)(?:(['\\^])(.*))?"};
  smatch sm;
  regex_match(s, sm, rx);
  vector<string> matches;
  vector<size_t> positions;
  FactorSP lhs{nullptr};
  TermSP rhs{nullptr};
  if(sm.size()>1)
  {
    nonEmptyMatchesAndPositions(sm, matches, positions);
    if(matches.size()==1)
      return parseAtom(matches[0], positions[0]);

    else if(matches.size()==2 && matches[1][0] == '\'')
      return parseDerivative(matches[0], positions[0]);

    else if(matches.size()==3 && matches[1][0] == '^')
      return parsePow(matches[0], matches[2], positions[0]);

    throw runtime_error("disformed factor");
  }
  throw runtime_error{"Malformed Factor: " + s};
}
    
AtomSP Parser::parseAtom(const string &s, size_t column)
{
  double value{0}; 
  try
  { 
    value = stod(s); 
    return make_shared<Real>(value, currline, column);
  }
  catch(const invalid_argument &e)
  {
    return make_shared<Symbol>(s, currline, column);
  }
  throw runtime_error{"Malformed Atom: " + s};
}
    
DifferentiateSP Parser::parseDerivative(const string &s, size_t column)
{
  return 
    make_shared<Differentiate>(
        make_shared<Symbol>(s, currline, column), currline, column);
}
    
PowSP Parser::parsePow(const string &lower, const string &upper, size_t column)
{
  return make_shared<Pow>(
      make_shared<Symbol>(lower, currline, column), 
      parseAtom(upper, column), 
      currline, column
    );
}

string Parser::parseName(
    string::const_iterator &begin, string::const_iterator end,
    size_t column, DiagnosticReport &dr)
{
  regex rx{"([a-zα-ωΑ-ΩA-Z_][a-zα-ωΑ-ΩA-Z0-9_]*)"};
  smatch sm;
  regex_search(begin, end, sm, rx);
  if(sm.empty())
    dr.diagnostics.push_back({
        Diagnostic::Level::Error,
        "Malformed name: `" + string(begin, end) + "`",
        currline, column
        });

  begin = sm[0].second;
  return sm[0];
}
    
size_t Parser::parsePrimes(std::string::const_iterator &begin, 
    std::string::const_iterator end, size_t column, DiagnosticReport &dr)
{
  for(auto it=begin; it!= end; ++it)
  {
    if(*it != '\'') 
    {
      dr.diagnostics.push_back({
          Diagnostic::Level::Error,
          "Expected prime or nothing: `" + string(begin,end) + "`" ,
          currline, column
          });
      return 0;
    }
  }
  size_t order = end-begin;
  begin = end;
  return order;
}

VarRefSP Parser::parseVRef(
    ComponentSP csp, 
    string::const_iterator &begin, string::const_iterator end, 
    size_t column, DiagnosticReport &d)
{
  string name = parseName(begin, end, column, d);
  if(d.catastrophic()) throw CompilationError{d};
  if(begin == end) return make_shared<VarRef>(csp, name); 

  size_t order = parsePrimes(begin, end, column + (end-begin), d);
  if(d.catastrophic()) 
  {
    d.diagnostics.push_back({
        Diagnostic::Level::Info,
        "At symbol " + name,
        currline, column
        });
    throw CompilationError{d};
  }
  return make_shared<DVarRef>(csp, name, order);
}

ComponentSP Parser::parseComponent(const string &s)
{
  smatch sm;
  regex_match(s, sm, comprx());
  if(sm.size() < 3) throw runtime_error("disformed component instance");
  
  auto cp = make_shared<Component>(
              make_shared<Symbol>(sm[1], currline, sm.position(1)),
              make_shared<Symbol>(sm[2], currline, sm.position(2)),
              currline, sm.position(1)
          );

  vector<string> params;
  vector<size_t> param_pos;
  size_t sp = sm.position(3);
  extractParams(sm[3], params, param_pos, sp);
  for(size_t i=0; i<params.size(); ++i)
  {
    string &p = params[i];
    if(p.find(":") != string::npos)
    {
      auto ps = split(p, ':');
      cp->params[make_shared<Symbol>(ps[0], currline, param_pos[i])] = 
        make_shared<Real>(stod(ps[1]), currline, param_pos[i]);
    }
    else if(p.find("|") != string::npos)
    {
      auto ps = split(p, '|');

      DiagnosticReport dr;
      auto it = ps[0].cbegin();
      VarRefSP vr = parseVRef(cp, it, ps[0].cend(), param_pos[i], dr);
      if(dr.catastrophic())
        throw CompilationError(dr);

      cp->initials[vr] = 
        make_shared<Real>(stod(ps[1]), currline, param_pos[i]);
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
