#include "Cypress/Core/Elements.hxx"
#include <iostream>

using namespace cypress;
using std::ostream;
using std::cout;
using std::endl;
using std::shared_ptr;
using std::string;
using std::vector;
using std::pair;

ostream& cypress::operator << (ostream &o, const Decls &decls)
{
  o << "Decls" << endl;
  o << "  " << "[object]" << endl;
  for(auto obj : decls.objects) o << "    " << *obj << endl;

  o << "  " << "[controller]" << endl;
  for(auto controller : decls.controllers) o << "    " << *controller << endl;

  o << "  " << "[experiment]" << endl;
  for(auto expr : decls.experiments) o << "    " << *expr << endl;

  return o;
}

ostream& cypress::operator << (ostream &o, const Object &obj)
{
  o << "name=" << obj.name->value << ", ";

  o << "params={";
  for(size_t i=0; i<obj.params.size()-1; ++i) o << obj.params[i]->value << ",";

  o << obj.params.back()->value << "}" << endl;
  
  o << "      [equation]" << endl;
  for(const auto eqtn : obj.eqtns) showEqtn(o, *eqtn);

  return o;
}

ostream& cypress::operator << (ostream &o, const Controller &controller)
{
  o << "name=" << controller.name->value << ", ";
  o << "params={";
  for(size_t i=0; i<controller.params.size()-1; ++i)
    o << controller.params[i]->value << ",";

  o << controller.params.back()->value << "}" << endl;

  o << "      [equation]" << endl;
  for(const auto eqtn : controller.eqtns) showEqtn(o, *eqtn);
  return o;
}

ostream& cypress::operator << (ostream &o, const Experiment &expr)
{
  o << "name=" << expr.name->value << endl;
  o << "      [components]" << endl;
  for(auto cp : expr.components)
    o << *cp;
  o << "      [connections]" << endl;
  for(auto cx : expr.connections)
    o << *cx;
  return o;
}

ostream& cypress::operator << (ostream &o, const Component &cp)
{
  o << "        "
    << "kind=" << cp.kind->value << " name=" << cp.name->value << " prams={";

  //vector<pair<SymbolSP, RealSP>> vparams;
  vector<pair<SymbolSP, string>> vparams;
  vparams.reserve(cp.params.size());

  for(auto pr : cp.params) vparams.push_back(pr);
  for(auto it = vparams.begin(); it != vparams.end()-1; ++it)
    //o << it->first->value << ":" << it->second->value << ", ";
    o << it->first->value << ":" << it->second << ", ";
    
  //o << vparams.back().first->value << ":" << vparams.back().second->value;
  o << vparams.back().first->value << ":" << vparams.back().second;


  o << "}" << endl;

  return o;
}

ostream& cypress::operator << (ostream &o, const Connection &lnk)
{
  o << "        "
    << *lnk.from << " > " << *lnk.to << endl;

  return o;
}

ostream& cypress::operator << (ostream &o, const Connectable &lkb)
{
  switch(lkb.kind())
  {
    case Connectable::Kind::Component:
      o << static_cast<const ComponentRef &>(lkb).name->value;
      break;
    case Connectable::Kind::SubComponent:
      o << static_cast<const SubComponentRef &>(lkb).name->value
        << "."
        << static_cast<const SubComponentRef &>(lkb).subname->value;
      break;
    case Connectable::Kind::AtoD:
      o << "|" << static_cast<const AtoD &>(lkb).rate << "|";
      break;
  }
  return o;
}

void cypress::showEqtn(ostream &o, const Equation &eqtn)
{
  o << string(8, ' ') << "=" << endl;
  showExpr(10, o, *eqtn.lhs);
  showExpr(10, o, *eqtn.rhs);
}

void cypress::showExpr(size_t indent, ostream &o, const Expression &expr)
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

    case Expression::Kind::CVar:
      o << I << "[:]" << endl;
      showExpr(indent+2, o, *static_cast<const CVar &>(expr).value);
      break;

    case Expression::Kind::CCVar:
      o << I << "{:}" << endl;
      showExpr(indent+2, o, *static_cast<const CCVar &>(expr).value);
      break;

    case Expression::Kind::BoundVar:
    {
      auto &bv = static_cast<const BoundVar &>(expr);
      switch(bv.bound->kind)
      {
        case Bound::Kind::LT: o << I << "<" << endl; break;
        case Bound::Kind::AbsLT: o << I << "|<" << endl; break;
        case Bound::Kind::GT: o << I << ">" << endl; break;
        case Bound::Kind::AbsGT: o << I << "|>" << endl; break;
      }
      showExpr(indent+2, o, *bv.value);
      break;
    }

    case Expression::Kind::IOVar:
    {
      auto &iv = static_cast<const IOVar &>(expr);
      switch(iv.iokind)
      {
        case IOVar::IOKind::Input: o << I << "|In|" << endl; break;
        case IOVar::IOKind::Output: o << I << "|Out|" << endl; break;
      }
      showExpr(indent+2, o, *iv.value);
      break;
    }

    case Expression::Kind::Symbol: o << I 
                                     << static_cast<const Symbol&>(expr).value 
                                     << endl; break;

    case Expression::Kind::Real: o << I
                                   << static_cast<const Real&>(expr).value
                                   << endl; break;
    case Expression::Kind::SubExpression: 
      showExpr(indent+2, o, *static_cast<const SubExpression&>(expr).value);
      break;
  }
}
