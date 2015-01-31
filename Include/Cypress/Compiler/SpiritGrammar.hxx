#ifndef CYPRESS_COMPILE_GRAMMAR_HXX
#define CYPRESS_COMPILE_GRAMMAR_HXX

#define BOOST_SPIRIT_UNICODE

#include "Dubious.hxx"

#include <boost/spirit/home/x3/char/unicode.hpp>

namespace x3 = boost::spirit::x3;

namespace cypress { namespace compile {

x3::rule<class Declarations> const declarations("Declarations");
x3::rule<class ObjectDecl> const object_decl("ObjectDecl");
x3::rule<class ControllerDecl> const controller_decl("ControllerDecl");
x3::rule<class ExperimentDecl> const experiment_decl("ExperimentDecl");

x3::rule<class Name, std::vector<char>> const name("Name");
x3::rule<class ParameterList> const parameter_list("ParameterList");

x3::rule<class Equation> const equation("Equation");
x3::rule<class Expression> const expression("Expression");
x3::rule<class Term> const term("Term");
x3::rule<class Factor> const factor("Factor");

using x3::uint_;
using x3::standard_wide::alpha;
using x3::alnum;
using x3::char_;

/*
bool gralpha(int ch)
{
  return std::isalpha(ch) || std::isalpha(ch, std::locale("el_GR.UTF-8"));
}
*/

//auto gralpha = x3::char_class<boost::spirit::char_encoding::standard, x3::greek_tag>();


auto const declarations_def = 
  *(object_decl | controller_decl | experiment_decl);

auto const name_def = 
  alpha >> *(alnum|char_('_'));

auto const object_decl_def = 
  "Object" >> name >> -(parameter_list) >> *(equation);

auto const controller_decl_def = 
  x3::string("Controller"); //TODO

auto const experiment_decl_def = 
  x3::string("Experiment"); //TODO

auto const parameter_list_def = 
  '(' >> -( name >> *(',' >> name)) >> ')';

auto const equation_def = 
  expression >> '=' >> expression;

auto const expression_def = 
  term >> *( ('+' >> term) | ('-' >> term));

auto const term_def = 
  factor >> *( ('*' >> factor) | ('/' >> factor));

auto const factor_def = 
  uint_ | '(' >> expression >> ')' | ('-' >> factor) | (name >> '\'') | name;
  //| (factor >> '^' >> factor);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
BOOST_SPIRIT_DEFINE(
    declarations = declarations_def,
    object_decl = object_decl_def,
    controller_decl = controller_decl_def,
    experiment_decl = experiment_decl_def,
    name = name_def,
    parameter_list = parameter_list_def,
    equation = equation_def,
    expression = expression_def,
    term = term_def,
    factor = factor_def);
#pragma clang diagnostic pop

auto experiment_grammar = declarations;

}}


#endif
