add_library(CypressSim 
  SHARED 
  Sim.cxx SimEx.cxx 
  ComputeNode.cxx Simutron.cxx
  Sensor.cxx)

target_link_libraries(CypressSim 
  CypressCore
  CypressControl
  /usr/local/lib/libsundials_ida.so 
  /usr/local/lib/libsundials_nvecparallel.so
  /usr/local/lib/libsundials_nvecserial.so
  /usr/local/lib/libmpi.so)

#add_executable(CySimExec SimEx.cxx)
#target_link_libraries(CySimExec CypressSim)

#add_executable(DummyResid DummyResid.cxx ComputeNodeMain.cxx)
#add_executable(DummyResidDirect DummyResid.cxx ComputeNodeDirectMain.cxx)
#target_link_libraries(DummyResid CypressSim CypressCore /usr/local/lib/libsundials_ida.so /usr/local/lib/libsundials_nvecparallel.so)
#target_link_libraries(DummyResidDirect CypressSim CypressCore)
