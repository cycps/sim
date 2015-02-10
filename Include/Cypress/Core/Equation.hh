namespace cypress {

template<class BinOp>
void EqtnParametizer::apply(std::shared_ptr<BinOp> x)
{
  if(x->lhs->kind() == Expression::Kind::Symbol)
  {
    auto symb = std::static_pointer_cast<Symbol>(x->lhs);
    if(symb->value == symbol_name)
    {
      x->lhs = std::make_shared<Real>(value);
    }
  }
}

}
