//Cypress Rotor Controller Simulation

Object Rotor(H)
  w = theta'
  w' = tau - H*w^2

Simulation RotorSim
  Rotor rotor(H:2.5)
  Actuator a(Min:-2, Max:2, DMin:-0.5, DMax:0.5)

  a.u ~ rotor.tau

