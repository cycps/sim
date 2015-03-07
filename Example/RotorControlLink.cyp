//Cypress Rotor Controller Experiment

//A very simple mathematical model of a link
//there is a send (tx) and receive (rx) buffer

Link SLink(lt, bw, cc)
  srx = stx(t - lt)
  stx = ix |< bw
  qd =  ix - bw |> 0
  tp = (srx' + stx')/2
  rx[0:srx] = tx[0:srx](t - lt)

Object Rotor(H)
  ω = θ'
  a = τ - H*ω^2
  a = ω'

Controller RotorSpeedController(ωt)
  a' = ωt - ω
  τ = a

Experiment RotorControl
  Rotor rotor(H:2.5, ω|0, θ|0, θ'|0, a|2.1, τ|2.1, ω'|0)
  RotorSpeedController ctrl(ωt:100, τ|0, a'|0)
  SLink lnk0(lt:0.005, bw:100E6, cc:1)
  SLink lnk1(lt:0.010, bw:250E6, cc:1)

  rotor.ω > |0.01| > lnk0 > ctrl.ω 
  ctrl.τ > lnk1 > rotor.τ

