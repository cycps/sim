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
template<class Lifter>
template<class BinOp>
void VarLifter<Lifter>::liftBinary(std::shared_ptr<BinOp> x, 
    std::string symbol_name)
{
  lift(&(x->lhs), symbol_name);
  lift(&(x->rhs), symbol_name);
}

template<class Lifter>
template<class UnOp>
void VarLifter<Lifter>::liftUnary(std::shared_ptr<UnOp> x, 
    std::string symbol_name)
{
  lift(&(x->value), symbol_name);
}

template<class Lifter>
template<class Kinded>
void VarLifter<Lifter>::lift(std::shared_ptr<Kinded> *x, 
    std::string symbol_name)
{
  std::shared_ptr<Lifter> lifted{nullptr};
  if((*x)->kind() == Expression::Kind::Symbol)
  {
    auto symb = std::static_pointer_cast<Symbol>(*x);
    if(lifts_vars && symb->value == symbol_name)
    {
      lifted = std::make_shared<Lifter>(symb);
      *x = lifted;
      onlift(lifted);
    }
  }
  else if(lifts_derivs && ((*x)->kind() == Expression::Kind::Differentiate))
  {
    auto dif = std::static_pointer_cast<Differentiate>(*x);
    if(dif->arg->value == symbol_name)
    {
      lifted = std::make_shared<Lifter>(dif);
      *x = lifted;
      onlift(lifted);
    }
  }
}

//VarLifter ------------------------------------------------------------------
template<class Lifter>
void VarLifter<Lifter>::visit(EquationSP ep)
{
  liftBinary(ep, symbol_name);
}

template<class Lifter>
void VarLifter<Lifter>::visit(AddSP ap)
{
  liftBinary(ap, symbol_name);
}

template<class Lifter>
void VarLifter<Lifter>::visit(SubtractSP sp)
{
  liftBinary(sp, symbol_name);
}

template<class Lifter>
void VarLifter<Lifter>::visit(MultiplySP mp)
{
  liftBinary(mp, symbol_name);
}

template<class Lifter>
void VarLifter<Lifter>::visit(DivideSP dp)
{
  liftBinary(dp, symbol_name);
}

template<class Lifter>
void VarLifter<Lifter>::visit(PowSP pp)
{
  liftBinary(pp, symbol_name);
}

template<class Lifter>
void VarLifter<Lifter>::visit(SubExpressionSP sp)
{
  liftUnary(sp, symbol_name);  
}

}
