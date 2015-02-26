#include "Cypress/Sim/SimEx.hxx"
#include <iostream>
#include <sstream>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <vector>
#include <stdexcept>

using namespace::cypress;
using std::cout;
using std::endl;
using std::string;
using std::stringstream;
using std::ifstream;
using std::vector;
using std::runtime_error;
using std::stol;
using std::stod;

SimEx::SimEx(size_t neq, double rtol, double satol)
  : neq{neq},
    rtol{rtol},
    satol{satol}
{
}

SimEx::SimEx(string source)
{
  vector<string> lines;
  boost::split(lines, source, boost::is_any_of("\n"));
  neq = parseNEQ(lines[0]);
  rtol = parseRTOL(lines[1]);
  satol = parseSATOL(lines[2]);

  startupReport();
}

template<class T, class F>
T getNamedParam(string ln, F f)
{
  vector<string> parts;
  boost::split(parts, ln, boost::is_any_of(":"));
  if(parts.size() != 2)
    throw runtime_error{"Malformed Pair"};
  return f(parts[1]);
}

size_t SimEx::parseNEQ(string ln)
{
  return getNamedParam<size_t>(ln, [](string s){return stol(s);});
}

double SimEx::parseRTOL(std::string ln)
{
  return getNamedParam<double>(ln, [](string s){return stod(s);});
}

double SimEx::parseSATOL(std::string ln)
{
  return getNamedParam<double>(ln, [](string s){return stod(s);});
}

string SimEx::toString()
{
  stringstream ss;

  ss 
    << "neq:" << neq << endl
    << "rtol:" << rtol << endl
    << "satol:" << satol << endl;

  return ss.str();
}


void SimEx::startupReport()
{
  cout 
    << "Cypress SimEX" << endl
    << "~~~~~-----~~~---~~--~-" << endl;

  cout 
    << "Number of Equations: " << neq << endl
    << "Relative Error Tolerance: " << rtol << endl;

  cout
    << "Uniform Absolute Error Tolerance: " << satol << endl;
}

void SimEx::run()
{

}

int main(int argc, char **argv)
{
  if(argc < 2) 
  {
    cout << "usage: CySimExex input" << endl;
    return 1;
  }

  string input = argv[1];

  ifstream ifs(input);
  if(!ifs.good())
  {
    cout << "Bad Input: " << input << endl;
    return 1;
  }

  string src((std::istreambuf_iterator<char>(ifs)),
              std::istreambuf_iterator<char>());
    
  SimEx sx(src);
  sx.run();
  return 0;
}
