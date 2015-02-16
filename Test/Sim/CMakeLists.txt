add_executable(RotorSundialsTest RotorSundialsTest.cxx)
target_link_libraries(RotorSundialsTest 
  gtest 
  gtest_main 
  /usr/local/lib/libsundials_ida.dylib 
  /usr/local/lib/libsundials_nvecserial.dylib)

add_executable(RotorSundialsTestKryPar RotorSundialsTestKryPar.cxx)
target_link_libraries(RotorSundialsTestKryPar
  /usr/local/lib/libmpi.dylib
  /usr/local/lib/libsundials_ida.dylib 
  /usr/local/lib/libsundials_nvecparallel.dylib)
