#ifndef CYPRESS_COMPILE_PARSER_HXX
#define CYPRESS_COMPILE_PARSER_HXX

#include "AST.hxx"

#include <string>
#include <vector>

namespace cypress { namespace compile {

enum class DeclType { Object, Controller, Experiment };
enum class LineType { Decl, Code, Comment, Empty, SomethingElse };

class Parser
{
  public:
    explicit Parser(std::string source);
    void run();

    std::vector<std::shared_ptr<Object>> objects;

  private:
    std::vector<std::string> lines;
    std::string source;

    LineType classifyLine(const std::string &, DeclType &dt);
    bool isDecl(const std::string &, DeclType &);
    bool isCode(const std::string &);
    bool isComment(const std::string &);
    bool isEmpty(const std::string &);
    bool isEqtn(const std::string &);

    size_t parseDecl(size_t at, DeclType dt);
    size_t parseObject(size_t at);
    size_t parseController(size_t at);
    size_t parseExperiment(size_t at);
    Equation parseEqtn(const std::string &);
    std::shared_ptr<Expression> parseExpr(const std::string &);
};

std::vector<std::string> &
split(const std::string &s, char delim, std::vector<std::string> &elems);

std::vector<std::string> 
split(const std::string &s, char delim);

}}

#endif
