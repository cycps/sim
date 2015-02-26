//Cypress Rotor Controller Experiment

Object Rotor(H)
  ω = θ'
  a = τ - H*ω^2
  a = ω'

Controller RotorSpeedControler(ωt)
  a' = ωt - ω
  τ = a

Experiment RotorControl
  Rotor rotor(H:2.5, ω|1, θ|2, θ'|3, a|4, ω'|5)
  RotorSpeedControler ctrl(ωt:100, τ|0, a'|0)
  Link lnk0(Latency:5, Bandwidth:100)
  Link lnk1(Latency:10, Bandwidth:100)

  rotor.ω > |0.01| > lnk0 > ctrl.ω 
  ctrl.τ > lnk1 > rotor.τ

