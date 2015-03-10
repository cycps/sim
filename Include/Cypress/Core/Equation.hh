namespace cypress {

//EqtnParametizer -------------------------------------------------------------
template<class BinOp>
void EqtnParametizer::apply(std::shared_ptr<BinOp> x)
{
  parametize(&(x->lhs));
  parametize(&(x->rhs));
}

template<class Kinded>
void EqtnParametizer::parametize(std::shared_ptr<Kinded> *x)
{
  if((*x)->kind() == Expression::Kind::Symbol)
  {
    auto symb = std::static_pointer_cast<Symbol>(*x);
    if(symb->value == symbol_name)
    {
      *x = std::make_shared<Real>(value, symb->line, symb->column);
    }
  }
}

//CVarLifter ------------------------------------------------------------------
template<class Lifter, class BinOp>
void liftBinary(std::shared_ptr<BinOp> x, std::string symbol_name, 
    bool lift_deriv)
{
  lift<Lifter>(&(x->lhs), symbol_name, lift_deriv);
  lift<Lifter>(&(x->rhs), symbol_name, lift_deriv);
}

template<class Lifter, class UnOp>
void liftUnary(std::shared_ptr<UnOp> x, std::string symbol_name, 
    bool lift_deriv)
{
  lift<Lifter>(&(x->value), symbol_name, lift_deriv);
}

template<class Lifter, class Kinded>
void lift(std::shared_ptr<Kinded> *x, std::string symbol_name, bool lift_deriv)
{
  if((*x)->kind() == Expression::Kind::Symbol)
  {
    auto symb = std::static_pointer_cast<Symbol>(*x);
    if(symb->value == symbol_name)
    {
      *x = std::make_shared<Lifter>(symb);
    }
  }
  else if(lift_deriv && ((*x)->kind() == Expression::Kind::Differentiate))
  {
    auto dif = std::static_pointer_cast<Differentiate>(*x);
    if(dif->arg->value == symbol_name)
    {
      *x = std::make_shared<Lifter>(dif);
    }
  }
}

}
