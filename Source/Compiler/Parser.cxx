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

Parser::Parser(string source)
  : source{source}
{}

void Parser::run()
{
  split(source, '\n', lines);

  for(size_t i=0; i<lines.size(); ++i)
  {
    DeclType dt;
    LineType lt = classifyLine(lines[i], dt);
    switch(lt)
    {
      case LineType::Decl : i += parseDecl(i, dt); break;
      case LineType::Code : 
        throw runtime_error("[" + to_string(i+1) + "] orphan code");
      case LineType::Comment : break;
      case LineType::Empty : break;
      case LineType::SomethingElse : 
        throw runtime_error(
            "[" + to_string(i+1) + "] not sure what this line is doing");
    }
  }
}

size_t Parser::parseDecl(size_t at, DeclType dt)
{
  switch(dt)
  {
    case DeclType::Object : return parseObject(at);
    case DeclType::Controller: return parseController(at);
    case DeclType::Experiment: return parseExperiment(at);
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

size_t Parser::parseObject(size_t at)
{
  cout << "Parsing Object : `" << lines[at] << "`" << endl;
  size_t idx = at+1;
  while(isCode(lines[idx]) || isEmpty(lines[idx]))
  { 
    if(isEmpty(lines[idx])) cout << "o" << endl;
    else if(isComment(lines[idx])) cout << "/" << endl;
    else 
    {
      if(isEqtn(lines[idx])) 
      {
        Equation eqtn = parseEqtn(lines[idx]);
        cout << "e" << endl;
      }
      else cout << "." << endl; 
    }
    ++idx; 
    if(idx >= lines.size()) break;
  }
  return idx - at - 1;
}
    
size_t Parser::parseController(size_t at)
{
  cout << "Parsing Controller : `" << lines[at] << "`" << endl;
  size_t idx = at+1;
  while(isCode(lines[idx]) || isEmpty(lines[idx]))
  { 
    if(isEmpty(lines[idx])) cout << "o" << endl;
    else if(isComment(lines[idx])) cout << "/" << endl;
    else 
    {
      if(isEqtn(lines[idx])) cout << "e" << endl;
      else cout << "." << endl; 
    }
    ++idx; 
    if(idx >= lines.size()) break;
  }
  return idx - at - 1;
}

size_t Parser::parseExperiment(size_t at)
{
  cout << "Parsing Experiment : `" << lines[at] << "`" << endl;
  size_t idx = at+1;
  while(isCode(lines[idx]) || isEmpty(lines[idx]))
  { 
    if(isEmpty(lines[idx])) cout << "o" << endl;
    else if(isComment(lines[idx])) cout << "/" << endl;
    else 
    {
      if(isEqtn(lines[idx])) cout << "e" << endl;
      else cout << "." << endl; 
    }
    ++idx; 
    if(idx >= lines.size()) break;
  }
  return idx - at - 1;
}

bool Parser::isDecl(const string &s, DeclType &dt)
{
  string Object{"Object"}, 
         Controller{"Controller"},
         Experiment{"Experiment"};

  if(s.compare(0, Object.length(), Object) == 0)
  {
    dt = DeclType::Object;
    return true;
  }
  if(s.compare(0, Controller.length(), Controller) == 0)
  {
    dt = DeclType::Controller;
    return true;
  }
  if(s.compare(0, Experiment.length(), Experiment) == 0)
  {
    dt = DeclType::Experiment;
    return true;
  }
  
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

Equation Parser::parseEqtn(const string &s)
{
  regex rx{"(.*)=(.*)"};
  smatch sm;
  regex_match(s, sm, rx);
  
  Equation eqtn;
  if(sm.size() != 3) throw runtime_error("disformed equation");
  eqtn.lhs = parseExpr(sm[1]);
  eqtn.rhs = parseExpr(sm[2]);

  return eqtn;
}
    
shared_ptr<Expression> Parser::parseExpr(const std::string &s)
{
  //remove white space
  std::string s_{s};
  s_.erase(remove_if(s_.begin(), s_.end(), isspace), s_.end());
  regex rx{"([a-zA-Zα-ωΑ-Ω0-9\\*\\^ ]+)(?:([\\+\\-])([ a-zA-Zα-ωΑ-Ω0-9\\*\\^]+))*"};
  smatch sm;
  regex_match(s_, sm, rx);
  cout << "expr match '" << s_ << "'" << endl;
  vector<string> matches;
  if(sm.size()>1)
  {
    copy_if(sm.begin()+1, sm.end(), back_inserter(matches), [](const string &m){ return !m.empty(); });
    for(const string &m : matches)
        cout << "`" << m << "`" << endl;
  }

  return make_shared<Add>();
}

std::vector<std::string> &
cypress::compile::split( const std::string &s, char delim, 
    std::vector<std::string> &elems) 
{
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}


std::vector<std::string> 
cypress::compile::split(const std::string &s, char delim) 
{
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}
