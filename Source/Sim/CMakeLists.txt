add_library(CypressSim SHARED Sim.cxx SimEx.cxx ComputeNode.cxx)
target_link_libraries(CypressSim 
  CypressCore
  /usr/local/lib/libsundials_ida.dylib 
  /usr/local/lib/libsundials_nvecserial.dylib
  /usr/local/lib/libmpi.dylib)

add_executable(CySimExec SimEx.cxx)
target_link_libraries(CySimExec CypressSim)

add_executable(DummyResid DummyResid.cxx ComputeNodeMain.cxx)
target_link_libraries(DummyResid CypressSim CypressCore)
