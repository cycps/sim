//Cypress Rotor Controller Experiment

Object Rotor(H)
  w = theta'
  w' = tau - H*w^2
  //a = w'

Controller RotorSpeedController(wt)
  a' = wt - w
  a' |< 0.4
  tau = a

Experiment RotorControl
  Rotor rotor(H:2.5)
  //TODO: you are here, need to plumb initalization into controllers
  RotorSpeedController ctrl(wt:100, a'|100) 
  Link lnk0(Latency:5, Bandwidth:100)
  Link lnk1(Latency:10, Bandwidth:250)

  rotor.w > |0.01| > lnk0 > ctrl.w 
  ctrl.tau > lnk1 > rotor.tau

