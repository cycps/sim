//Cypress Rotor Controller Simulation

Object Rotor(H)
  w = theta'
  w' = tau - H*w^2

//TODO: Time to implement sensors
Simulation RotorSim
  Rotor rotor(H:2.5)
  Actuator atr_rt(Min:-2, Max:2, DMin:-0.5, DMax:0.5)
  Sensor sr_rw(Rate:10, Destination:localhost)

  atr_rt.u ~ rotor.tau
  rotor.w ~ sr_rw.y

