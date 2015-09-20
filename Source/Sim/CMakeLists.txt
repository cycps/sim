add_library(CypressSim 
  Sim.cxx SimEx.cxx 
  ComputeNode.cxx Simutron.cxx
  Sensor.cxx)

target_link_libraries(CypressSim 
  CypressCore
  CypressControl
  /usr/local/lib/libsundials_ida.a 
  /usr/local/lib/libsundials_nvecparallel.a
  /usr/local/lib/libsundials_nvecserial.a
  /usr/local/lib/libmpi.a)

#add_executable(CySimExec SimEx.cxx)
#target_link_libraries(CySimExec CypressSim)

add_executable(DummyResid DummyResid.cxx ComputeNodeMain.cxx)
add_executable(DummyResidDirect DummyResid.cxx ComputeNodeDirectMain.cxx)
target_link_libraries(DummyResid CypressSim pthread CypressCore)
target_link_libraries(DummyResidDirect CypressSim pthread CypressCore)

install(TARGETS CypressSim DummyResid DummyResidDirect
        ARCHIVE DESTINATION /usr/local/lib
        RUNTIME DESTINATION /usr/local/bin)
