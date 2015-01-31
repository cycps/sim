#include "Cypress/Compiler/Parser.hxx"
#include <iostream>

using namespace cypress::compile;

using std::string;
using std::cout;
using std::endl;

Parser::Parser(string source)
  : source_{source}
{}

void Parser::run()
{
  while(pos < source_.length())
  {
    while(pos < source_.length() && source_.substr(pos, 6) != "Object") nextLine();
    if(pos >= source_.length()) break;

    size_t obj_begin = line;
    nextLine();
    while(pos < source_.length() && std::isspace(source_[pos])) nextLine();
    size_t obj_end = line;

    if(pos < source_.length())
    {
      cout << "Object @(" << obj_begin << "," << obj_end << ")" << endl;
    }
  }
}

void Parser::nextLine()
{
  while(pos < source_.length() && source_[pos] != '\n') ++pos;  
  ++pos;
  ++line;
}
