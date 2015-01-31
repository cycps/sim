#ifndef CYPRESS_COMPILE_PARSER_HXX
#define CYPRESS_COMPILE_PARSER_HXX

#include "AST.hxx"

#include <string>
#include <vector>

namespace cypress { namespace compile {

class Parser
{
  public:
    explicit Parser(std::string source);
    void run();

    std::vector<std::shared_ptr<Object>> objects;

  private:
    std::string source_;
};

}}

#endif
