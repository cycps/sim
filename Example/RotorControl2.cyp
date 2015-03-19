//Cypress Rotor Control Experiment 2nd Go

Object Rotor(H)[tau]
  theta' = w
  w' = tau - H*w^2

Simulation RotorControl
  Rotor rotor(H:2.5)
  Actuator a(Min:-2, Max:2, MinRate:-0.5, MaxRate:0.5)

  a > rotor.tau
