#ifndef CYPRESS_COMPILE_GRAMMAR_HXX
#define CYPRESS_COMPILE_GRAMMAR_HXX

#include "Dubious.hxx"

namespace x3 = boost::spirit::x3;

namespace cypress { namespace compile {

x3::rule<class expression> const expression("expression");
x3::rule<class term> const term("term");
x3::rule<class factor> const factor("factor");

using x3::uint_;

auto const expression_def = 
  term >> *( ('+' >> term) | ('-' >> term));

auto const term_def = 
  factor >> *( ('*' >> factor) | ('/' >> factor));

auto const factor_def = 
  uint_ | '(' >> expression >> ')' | ('-' >> factor) | ('+' >> factor);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
BOOST_SPIRIT_DEFINE(
    expression = expression_def,
    term = term_def,
    factor = factor_def);
#pragma clang diagnostic pop

auto experiment = expression;

}}


#endif
