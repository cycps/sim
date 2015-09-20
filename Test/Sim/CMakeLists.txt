add_executable(RotorSundialsTest RotorSundialsTest.cxx)
target_link_libraries(RotorSundialsTest 
  gtest 
  gtest_main 
  /usr/local/lib/libsundials_ida.a
  /usr/local/lib/libsundials_nvecserial.a)

add_executable(RotorSundialsTestKryPar RotorSundialsTestKryPar.cxx)
target_link_libraries(RotorSundialsTestKryPar
  pthread
  /usr/local/lib/libmpi.a
  /usr/local/lib/libsundials_ida.a
  /usr/local/lib/libsundials_nvecparallel.a)
