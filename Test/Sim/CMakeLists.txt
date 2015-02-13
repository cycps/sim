add_executable(RotorSundialsTest RotorSundialsTest.cxx)
target_link_libraries(RotorSundialsTest gtest gtest_main /usr/local/lib/libsundials_ida.dylib /usr/local/lib/libsundials_nvecserial.dylib)
