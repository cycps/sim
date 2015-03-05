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
template<class BinOp>
void CVarLifter::applyBinary(std::shared_ptr<BinOp> x)
{
  lift(&(x->lhs));
  lift(&(x->rhs));
}

template<class UnOp>
void CVarLifter::applyUnary(std::shared_ptr<UnOp> x)
{
  lift(&(x->value));
}

template<class Kinded>
void CVarLifter::lift(std::shared_ptr<Kinded> *x)
{
  if((*x)->kind() == Expression::Kind::Symbol)
  {
    auto symb = std::static_pointer_cast<Symbol>(*x);
    if(symb->value == symbol_name)
    {
      *x = std::make_shared<CVar>(symb);
    }
  }
}

}
