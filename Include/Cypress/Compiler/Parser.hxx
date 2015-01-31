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
    size_t pos{0}, line{0};
    std::string source_;
    void nextLine();
};

}}

#endif
