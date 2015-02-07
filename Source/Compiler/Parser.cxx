#include "Cypress/Compiler/Parser.hxx"
#include <iostream>
#include <stdexcept>
#include <regex>
#include <string>
#include <vector>
#include <sstream>

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

Parser::Parser(string source)
  : source{source}
{}

std::shared_ptr<Decls> Parser::run()
{
  split(source, '\n', lines);
  auto decls = make_shared<Decls>();

  for(size_t i=0; i<lines.size(); ++i)
  {
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

size_t Parser::parseDecl(size_t at, DeclType dt, std::shared_ptr<Decls> decls)
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
    
LineType Parser::classifyLine(const std::string &s, DeclType &dt)
{
  if(isDecl(s, dt)) return LineType::Decl;
  if(isCode(s)) return LineType::Code;
  if(isComment(s)) return LineType::Comment;
  if(isEmpty(s)) return LineType::Empty;
  return LineType::SomethingElse;
}

shared_ptr<Object> Parser::parseObject(size_t at, size_t &lc)
{
  smatch sm;
  regex_match(lines[at], sm, objrx());
  auto object = make_shared<Object>(make_shared<Symbol>(sm[1]));

  //Parse the parameters
  string _pstr = sm[2];
  string pstr = string(_pstr.begin()+1, _pstr.end()-1);
  auto params = split(pstr, ',');

  transform(params.begin(), params.end(), back_inserter(object->params),
      [](const string &ps){ return make_shared<Symbol>(ps); });


  size_t idx = at+1;
  while(isCode(lines[idx]) || isEmpty(lines[idx]))
  { 
    if(isEmpty(lines[idx])) {}
    else if(isComment(lines[idx])) {}
    else 
    {
      if(isEqtn(lines[idx])) 
      {
        shared_ptr<Equation> eqtn = parseEqtn(lines[idx]);
        object->eqtns.push_back(eqtn);
      }
    }
    ++idx; 
    if(idx >= lines.size()) break;
  }
  lc = idx - at - 1;
  return object;
}
    
shared_ptr<Controller> Parser::parseController(size_t at, size_t &lc)
{
  smatch sm;
  regex_match(lines[at], sm, contrx());
  auto controller = make_shared<Controller>(make_shared<Symbol>(sm[1]));

  string _pstr = sm[2];
  string pstr = string(_pstr.begin()+1, _pstr.end()-1);
  auto params = split(pstr, ',');

  transform(params.begin(), params.end(), back_inserter(controller->params),
      [](const string &ps){ return make_shared<Symbol>(ps); });

  size_t idx = at+1;
  while(isCode(lines[idx]) || isEmpty(lines[idx]))
  { 
    if(isEmpty(lines[idx])) {}
    else if(isComment(lines[idx])) {}
    else 
    {
      if(isEqtn(lines[idx])) 
      {
        shared_ptr<Equation> eqtn = parseEqtn(lines[idx]);
        controller->eqtns.push_back(eqtn);
      }
    }
    ++idx; 
    if(idx >= lines.size()) break;
  }
  lc = idx - at - 1;
  return controller;
}

shared_ptr<Experiment> Parser::parseExperiment(size_t at, size_t &lc)
{
  smatch sm;
  regex_match(lines[at], sm, exprx());
  auto experiment = make_shared<Experiment>(make_shared<Symbol>(sm[1]));
  size_t idx = at+1;

  while(isCode(lines[idx]) || isEmpty(lines[idx]))
  { 
    if(isEmpty(lines[idx])) {}
    else if(isComment(lines[idx])) {}
    else 
    {
      if(regex_match(lines[idx], sm, comprx())) 
      {
        shared_ptr<Component> cp = parseComponent(lines[idx]);
        experiment->components.push_back(cp);
      }
      else if(regex_match(lines[idx], sm, lnkrx()))
      {
        vector<shared_ptr<Link>> lnks = parseLinkStmt(lines[idx]);
        experiment->links.insert(experiment->links.end(), lnks.begin(), lnks.end());
      }
    }
    ++idx; 
    if(idx >= lines.size()) break;
  }
  lc = idx - at - 1;
  return experiment;
}


vector<shared_ptr<Link>> Parser::parseLinkStmt(const string &s)
{
  auto links = split(s, '>');
  for(string &l : links)
    l.erase(remove_if(l.begin(), l.end(), isspace), l.end());

  vector<shared_ptr<Link>> lnks;
  smatch sm;
  for(size_t i=0; i<links.size()-1; ++i)
  {
    shared_ptr<Linkable> from, to;

    if(regex_match(links[i], sm, thingrx()))
    {
      from = make_shared<Thing>(make_shared<Symbol>(sm[1]));
    }
    else if(regex_match(links[i], sm, subthingrx()))
    {
      from = make_shared<SubThing>(
          make_shared<Symbol>(sm[1]),
          make_shared<Symbol>(sm[2]));
    }
    else if(regex_match(links[i], sm, atodrx()))
    {
      from = make_shared<AtoD>(stod(sm[1]));
    }
    else
      throw runtime_error{"disformed linkable : " + links[i]};

    
    if(regex_match(links[i+1], sm, thingrx()))
    {
      to = make_shared<Thing>(make_shared<Symbol>(sm[1]));
    }
    else if(regex_match(links[i+1], sm, subthingrx()))
    {
      to = make_shared<SubThing>(
          make_shared<Symbol>(sm[1]),
          make_shared<Symbol>(sm[2]));
    }
    else if(regex_match(links[i+1], sm, atodrx()))
    {
      to = make_shared<AtoD>(stod(sm[1]));
    }
    else
      throw runtime_error{"disformed linkable" + links[i+1]};

    lnks.push_back(make_shared<Link>(from, to));
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

shared_ptr<Equation> Parser::parseEqtn(const string &s)
{
  regex rx{"(.*)=(.*)"};
  smatch sm;
  regex_match(s, sm, rx);
  
  auto eqtn = make_shared<Equation>();
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
    
shared_ptr<Expression> Parser::parseExpr(const std::string &s)
{
  regex rx{"([a-zA-Zα-ωΑ-Ω0-9'/\\*\\^ ]+)(?:([\\+\\-])(.*))?"};
  smatch sm;
  regex_match(s, sm, rx);
  vector<string> matches;
  shared_ptr<Term> lhs{nullptr};
  shared_ptr<Expression> rhs{nullptr};
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
                case '+' : return make_shared<Add>(lhs, rhs);
                case '-' : return make_shared<Subtract>(lhs, rhs);
              }
    }
  }
  return nullptr;
}
    
std::shared_ptr<Term> Parser::parseTerm(const std::string &s)
{
  regex rx{"([a-zA-Zα-ωΑ-Ω0-9'\\^ ]+)(?:([\\*/])(.*))?"};
  smatch sm;
  regex_match(s, sm, rx);
  vector<string> matches;
  shared_ptr<Factor> lhs{nullptr};
  shared_ptr<Term> rhs{nullptr};
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
                case '*' : return make_shared<Multiply>(lhs, rhs);
                case '/' : return make_shared<Divide>(lhs, rhs);
              }
    }
  }
  return nullptr;
}
    
std::shared_ptr<Factor> Parser::parseFactor(const std::string &s)
{
  regex rx{"([a-zA-Zα-ωΑ-Ω0-9]+)(?:(['\\^])(.*))?"};
  smatch sm;
  regex_match(s, sm, rx);
  vector<string> matches;
  shared_ptr<Factor> lhs{nullptr};
  shared_ptr<Term> rhs{nullptr};
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
    
std::shared_ptr<Atom> Parser::parseAtom(const std::string &s)
{
  double value{0}; 
  try
  { 
    value = stod(s); 
    return make_shared<Real>(value);
  }
  catch(const invalid_argument &e)
  {
    return make_shared<Symbol>(s);
  }

  return nullptr;
}
    
std::shared_ptr<Differentiate> Parser::parseDerivative(const std::string &s)
{
  return make_shared<Differentiate>(make_shared<Symbol>(s));
}
    
shared_ptr<Pow> Parser::parsePow(const string &lower, const string &upper)
{
  return make_shared<Pow>(make_shared<Symbol>(lower), parseAtom(upper));
}

shared_ptr<Component> Parser::parseComponent(const string &s)
{
  smatch sm;
  regex_match(s, sm, comprx());
  if(sm.size() < 3) throw runtime_error("disformed component instance");
  
  auto cp = make_shared<Component>(
              make_shared<Symbol>(sm[1]),
              make_shared<Symbol>(sm[2]));

  string _pstr = sm[3];
  string pstr = string(_pstr.begin()+1, _pstr.end()-1);
  auto params = split(pstr, ',');
  for(const string &p : params)
  {
    auto ps = split(p, ':');
    ps[0].erase(remove_if(ps[0].begin(), ps[0].end(), isspace), ps[0].end());
    cp->params[make_shared<Symbol>(ps[0])] = make_shared<Real>(stod(ps[1]));
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
