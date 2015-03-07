//Cypress Rotor Controller Experiment

//Cypress Rotor Controller Experiment

//~ A very simple mathematical model of a link
//    tx, rx: send, receive buffers
//    txs, rxs, ixs: send, receive, input buffer sizes
//    lt, elt, bw: latency, effective latency, bandwidth
//    cc: congenstion coeffecient
//    qd: queue delay
//    tp: throughput

Link SLink(lt, bw, cc)
  rx[0:rxs] = tx[0:rxs](t - elt)
  rxs = txs(t - lt)
  txs = ixs |< bw
  elt = lt*qd*cc
  qd =  ixs - bw |> 0
  tp = (rxs' + txs')/2

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

