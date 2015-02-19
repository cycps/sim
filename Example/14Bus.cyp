//Cypress IEEE 14 Bus System Example

Bus
  v = v

Object Line(r, x)
  v = va - vb
  v = i*r + i'*x

Experiment IEEE14Bus

  Line l1_2(0.01938, 0.05917)
  Line l1_5(0.05403, 0.22304)
  Line l2_3(0.04699, 0.19797)
  Line l2_4(0.05811, 0.17632)
  Line l2_5(0.05695, 0.17388)
  Line l3_4(0.06701, 0.17103)
  Line l4_5(0.01335, 0.04211)
  Line l4_7(0, 0.20912)
  Line l4_9(0, 0.55618)
  Line l5_6(0, 0.25202)
  Line l6_11(0.09498, 0.19890)
  Line l6_12(0.12291, 0.25581)
  Line l6_13(0.06615, 0.13027)
  Line l7_8(0, 0.17615)
  Line l7_9(0, 0.11001)
  Line l9_10(0.03181, 0.08450)
  Line l9_14(0.12711, 0.27038)
  Line l10_11(0.08205, 0.19207)
  Line l12_13(0.22092, 0.19988)
  Line l13_14(0.17093, 0.34802)

  Bus b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12,
      b13, b14;

  l12.va ~ l15.va

  l12.va ~ b1.v
  l15.va ~ b1.v

  
