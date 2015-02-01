//Cypress Rotor Controller Experiment

Object Rotor(H)
  ω = θ'
  a = ω
  a = τ - H*ω^2

Controller RotorSpeedControler[ω](ωt) -> τ
  a' = ωt - ω
  τ = a

Experiment RotorControl
  Rotor rotor(H:2.5)
  RotorSpeedControler ctrl(ωt:100)
  Link lnk0(Latency:5, Bandwidth:100)
  Link lnk1(Latency:10, Bandwidth:100)

  rotor.ω ~[0.01]-> lnk0 --> ctrl.ω 
  ctrl.τ --> lnk1 -~> rotor.τ

