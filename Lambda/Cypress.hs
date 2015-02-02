{-# LANGUAGE FlexibleInstances, UndecidableInstances, GADTs, MultiParamTypeClasses #-}
--Cypress .-.-.-.- .-.-.-.-.-.-.- .-.-.-.- .-.-.-.-.-.-.- .-.-.-.- .-.-.-.-.-.-.

import Prelude hiding((-), (*), (^))

data Object = Object [(Equation Double)]

newtype Symbol = Symbol [Char] deriving Show
instance Expr Symbol () where
  liftExpr a = UExpr $ liftTerm a

instance TermT Symbol () where
  liftTerm a = UTerm $ liftFactor a

instance FactorT Symbol () where
  liftFactor a = SymbolFactor a

class Expr a b where
  liftExpr :: a -> Expression b

class TermT a b where
  liftTerm :: a -> Term b

class FactorT a b where
  liftFactor :: a -> Factor b

instance Expr (Expression k) k where
  liftExpr a = a

instance TermT (Term k) k where
  liftTerm a = a

data Equation a where 
  Equation :: (Expression a) -> (Expression a) -> Equation a

data Expression a where 
  Sum :: (Term a) -> (Term a) -> Expression a
  Difference :: (Term a) -> (Term a) -> Expression a
  UExpr :: (Term a) -> Expression a

data Term a where
  Product :: (Factor a) -> (Factor a) -> Term a
  Quotient :: (Factor a) -> (Factor a) -> Term a
  UTerm :: (Factor a) -> Term a

data Factor a where
  SymbolFactor :: Symbol -> Factor Symbol
  DerivativeFactor :: (Derivative a) -> Factor (Derivative a)
  ExponentFactor :: (Exponent a) -> Factor (Exponent a)
  RealFactor :: (Real b) => b -> Factor b

--Derivative
data Derivative a where 
  Derivative :: (Expression a) -> Derivative a

instance Expr (Derivative k) k where
  liftExpr a = UExpr $ UTerm $ DerivativeFactor a

--Exponent
data Exponent a where 
  Exponent :: (Expression a) -> (Expression a) -> Exponent a

instance Expr (Exponent k) k where
  liftExpr a = UExpr $ liftTerm a

instance TermT (Exponent k) k where
  liftTerm a = UTerm $ liftFactor a

instance FactorT (Exponent k) k where
  liftFactor a = ExponentFactor a

--Real
--instance FactorT Double where
--  liftFactor n = RealFactor n

--instance TermT Double where
--  liftTerm a = UTerm $ liftFactor a

--instance Expr Double where
--  liftExpr a = UExpr $ liftTerm a

--instance (Real a) => FactorT a where
--  liftFactor n = RealFactor $ (fromRational . toRational) n

--instance (Real a) => FactorT a where
--  liftFactor a = RealFactor (realToFrac a)

(<=>) :: (Expr a k, Expr b k) => a -> b -> Equation k
a <=> b = Equation (liftExpr a) (liftExpr b)

(-) :: (TermT a k, TermT b k) => a -> b -> Expression k
a - b = Difference (liftTerm a) (liftTerm b)

(*) :: (FactorT a k, FactorT b k) => a -> b -> Term k
a * b = Product (liftFactor a) (liftFactor b)


d :: Expr a k => a -> Derivative k
d expr = Derivative $ liftExpr expr

(^) :: (Expr a k, Expr b k) => a -> b -> Exponent k
a ^ b = Exponent (liftExpr a) (liftExpr b)

rotor :: Symbol -> Symbol -> Symbol -> Symbol -> Symbol -> Object
rotor ω θ a τ h = 
  Object [ 
    ω <=> (d θ),
    --a <=> (τ - (h*(ω^(2.0::Double))))
    --a <=> (τ - (h*(ω^2.0)))
    a <=> (ω^2.0)
  ]

