#include "Cypress/Compiler/AST.hxx"

using std::ostream;
using std::endl;
using std::shared_ptr;
using std::string;
using std::static_pointer_cast;

ostream& cypress::compile::operator << (ostream &o, const Decls &decls)
{
  o << "Decls" << endl;
  o << "  " << "[object]" << endl;
  for(shared_ptr<Object> obj : decls.objects)
    o << "    " << *obj << endl;

  o << "  " << "[controller]" << endl;
  for(shared_ptr<Controller> controller : decls.controllers)
    o << "    " << *controller << endl;

  o << "  " << "[experiment]" << endl;
  return o;
}

ostream& cypress::compile::operator << (ostream &o, const Object &obj)
{
  o << "name=" << obj.name->value << ", ";

  o << "params={";
  for(size_t i=0; i<obj.params.size()-1; ++i)
    o << obj.params[i]->value << ",";
  o << obj.params.back()->value << "}" << endl;
  
  o << "      [equation]" << endl;
  for(const shared_ptr<Equation> eqtn : obj.eqtns)
  {
    showEqtn(o, *eqtn);
  }

  return o;
}

ostream& cypress::compile::operator << (ostream &o, const Controller &controller)
{
  o << "      [equation]" << endl;
  for(const shared_ptr<Equation> eqtn : controller.eqtns)
  {
    showEqtn(o, *eqtn);
  }
  return o;
}

void cypress::compile::showEqtn(ostream &o, const Equation &eqtn)
{
  o << string(8, ' ') << "=" << endl;
  showExpr(10, o, *eqtn.lhs);
  showExpr(10, o, *eqtn.rhs);
}

void cypress::compile::showExpr(size_t indent, std::ostream &o, const Expression &expr)
{
  const string I(indent, ' ');
  switch(expr.kind())
  {
    case Expression::Kind::Add: 
      o << I << '+' << endl; 
      showExpr(indent+2, o, *static_cast<const GroupOp &>(expr).lhs);
      showExpr(indent+2, o, *static_cast<const GroupOp &>(expr).rhs);
      break;

    case Expression::Kind::Subtract: 
      o << I << '-' << endl; 
      showExpr(indent+2, o, *static_cast<const GroupOp &>(expr).lhs);
      showExpr(indent+2, o, *static_cast<const GroupOp &>(expr).rhs);
      break;

    case Expression::Kind::Multiply: 
      o << I << '*' << endl; 
      showExpr(indent+2, o, *static_cast<const RingOp &>(expr).lhs);
      showExpr(indent+2, o, *static_cast<const RingOp &>(expr).rhs);
      break;

    case Expression::Kind::Divide: 
      o << I << '/' << endl; 
      showExpr(indent+2, o, *static_cast<const RingOp &>(expr).lhs);
      showExpr(indent+2, o, *static_cast<const RingOp &>(expr).rhs);
      break;

    case Expression::Kind::Pow: 
      o << I << '^' << endl; 
      showExpr(indent+2, o, *static_cast<const Pow &>(expr).lhs);
      showExpr(indent+2, o, *static_cast<const Pow &>(expr).rhs);
      break;

    case Expression::Kind::Differentiate: 
      o << I << '\'' << endl; 
      showExpr(indent+2, o, *static_cast<const Differentiate &>(expr).arg);
      break;

    case Expression::Kind::Symbol: o << I 
                                     << static_cast<const Symbol&>(expr).value 
                                     << endl; break;

    case Expression::Kind::Real: o << I
                                   << static_cast<const Real&>(expr).value
                                   << endl; break;
  }
}

