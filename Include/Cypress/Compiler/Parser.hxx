#ifndef CYPRESS_COMPILE_PARSER_HXX
#define CYPRESS_COMPILE_PARSER_HXX

#include "Cypress/Core/Equation.hxx"
#include "Cypress/Core/Elements.hxx"

#include <string>
#include <vector>

namespace cypress { namespace compile {

enum class DeclType { Object, Controller, Experiment };
enum class LineType { Decl, Code, Comment, Empty, SomethingElse };

class Parser
{
  public:
    explicit Parser(std::string source);
    std::shared_ptr<Decls> run();

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

    size_t parseDecl(size_t at, DeclType dt, std::shared_ptr<Decls> decls);
    std::shared_ptr<Object> parseObject(size_t at, size_t &lc);
    std::shared_ptr<Controller> parseController(size_t at, size_t &lc);
    std::shared_ptr<Experiment> parseExperiment(size_t at, size_t &lc);
    std::shared_ptr<Equation> parseEqtn(const std::string &);
    std::shared_ptr<Expression> parseExpr(const std::string &);
    std::shared_ptr<Term> parseTerm(const std::string &);
    std::shared_ptr<Factor> parseFactor(const std::string &);
    std::shared_ptr<Atom> parseAtom(const std::string &);
    std::shared_ptr<Differentiate> parseDerivative(const std::string &);
    std::shared_ptr<Pow> parsePow(const std::string &, const std::string &);

    std::shared_ptr<Component> parseComponent(const std::string &);
    std::vector<ConnectionSP> parseConnectionStmt(const std::string &);
};

std::vector<std::string> &
split(const std::string &s, char delim, std::vector<std::string> &elems);

std::vector<std::string> 
split(const std::string &s, char delim);

}}

#endif
