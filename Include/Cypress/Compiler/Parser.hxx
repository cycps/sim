#ifndef CYPRESS_COMPILE_PARSER_HXX
#define CYPRESS_COMPILE_PARSER_HXX

#include "Cypress/Core/Equation.hxx"
#include "Cypress/Core/Elements.hxx"
#include "Cypress/Compiler/Diagnostics.hxx"
#include "Cypress/Core/Var.hxx"

#include <string>
#include <vector>

namespace cypress { namespace compile {

enum class DeclType { Object, Controller, Experiment, Link };
enum class LineType { Decl, Code, Comment, Empty, SomethingElse };

class Parser
{
  public:
    explicit Parser(std::string source, DiagnosticReport&);
    std::shared_ptr<Decls> run();

    std::vector<ObjectSP> objects;

    DiagnosticReport *dr;

  private:
    std::vector<std::string> lines;
    std::string source;

    size_t currline{0};

    LineType classifyLine(const std::string &, DeclType &dt);
    bool isDecl(const std::string &, DeclType &);
    bool isCode(const std::string &);
    bool isComment(const std::string &);
    bool isEmpty(const std::string &);
    bool isEqtn(const std::string &);

    size_t parseDecl(size_t at, DeclType dt, std::shared_ptr<Decls> decls);
    ObjectSP parseObject(size_t at, size_t &lc);
    ControllerSP parseController(size_t at, size_t &lc);
    LinkSP parseLink(size_t at, size_t &lc);
    ExperimentSP parseExperiment(size_t at, size_t &lc);
    EquationSP parseEqtn(const std::string &);
    ExpressionSP parseExpr(const std::string &);
    TermSP parseTerm(const std::string &);
    FactorSP parseFactor(const std::string &);
    AtomSP parseAtom(const std::string &, size_t column);
    DifferentiateSP parseDerivative(const std::string &, size_t column);
    PowSP parsePow(const std::string &, const std::string &, size_t column);

    void parseElementContent(ElementSP e, size_t at, size_t &lc);

    ComponentSP parseComponent(const std::string &);
    std::vector<ConnectionSP> parseConnectionStmt(const std::string &);

    std::string parseName(std::string::const_iterator &begin, 
        std::string::const_iterator end, size_t column);

    VarRefSP parseVRef(ComponentSP csp, std::string::const_iterator &begin, 
        std::string::const_iterator end, size_t column);

    size_t parsePrimes(std::string::const_iterator &begin,
        std::string::const_iterator end, size_t column);
};

std::vector<std::string> &
split(const std::string &s, char delim, std::vector<std::string> &elems);

std::vector<std::string> 
split(const std::string &s, char delim);

}}

#endif
