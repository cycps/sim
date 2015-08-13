add_executable(RotorSundialsTest RotorSundialsTest.cxx)
target_link_libraries(RotorSundialsTest 
  gtest 
  gtest_main 
  /usr/local/lib/libsundials_ida.so
  /usr/local/lib/libsundials_nvecserial.so)

add_executable(RotorSundialsTestKryPar RotorSundialsTestKryPar.cxx)
target_link_libraries(RotorSundialsTestKryPar
  /usr/local/lib/libmpi.so
  /usr/local/lib/libsundials_ida.so
  /usr/local/lib/libsundials_nvecparallel.so)
