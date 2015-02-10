#ifndef CYPRESS_EQTN
#define CYPRESS_EQTN

#include "Cypress/Compiler/AST.hxx"

namespace cypress {

std::shared_ptr<compile::Equation> 
setToZero(std::shared_ptr<compile::Equation>);

void
setEqtnsToZero(std::shared_ptr<compile::Element>);

std::shared_ptr<compile::Element>
qualifyEqtns(std::shared_ptr<compile::Element>);

struct EqtnQualifier : public compile::Visitor
{
  std::shared_ptr<compile::Component> qual{nullptr};
  void setQualifier(std::shared_ptr<compile::Component>);
  void visit(std::shared_ptr<compile::Symbol>) override;
  void run(std::shared_ptr<compile::Equation>);
};

struct EqtnParametizer : public compile::Visitor
{
  std::string symbol_name;
  double value;
  
  void visit(std::shared_ptr<compile::Add>) override;
  void visit(std::shared_ptr<compile::Subtract>) override;
  void visit(std::shared_ptr<compile::Multiply>) override;
  void visit(std::shared_ptr<compile::Divide>) override;
  void visit(std::shared_ptr<compile::Pow>) override;
};

void applyParameter(std::shared_ptr<compile::Equation>, std::string symbol_name, double value);

}

#endif
