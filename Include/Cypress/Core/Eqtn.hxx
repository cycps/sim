#ifndef CYPRESS_EQTN
#define CYPRESS_EQTN

#include "Cypress/Compiler/AST.hxx"

namespace cypress {

std::shared_ptr<compile::Equation> 
setToZero(std::shared_ptr<compile::Equation>);

std::shared_ptr<compile::Element>
qualifyEqtns(std::shared_ptr<compile::Element>);

}

#endif
