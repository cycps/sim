//Cypress Rotor Controller Simulation

Object Rotor(H)
  w' = tau - H*w^2
  w = theta'

Simulation RotorSim
  Rotor rotor(H:1.5)
  Actuator atr_rt(Min:-15, Max:15, DMin:-0.7, DMax:0.7)
  Sensor sr_rw(Rate:10, Destination:localhost)

  atr_rt.u ~ rotor.tau
  rotor.w ~ sr_rw.y

