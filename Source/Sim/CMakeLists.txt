add_library(CypressSim SHARED Sim.cxx SimEx.cxx)
target_link_libraries(CypressSim 
  CypressCore
  /usr/local/lib/libsundials_ida.dylib 
  /usr/local/lib/libsundials_nvecserial.dylib)

add_executable(CySimExec SimEx.cxx)
target_link_libraries(CySimExec 
  /usr/local/lib/libsundials_ida.dylib 
  /usr/local/lib/libsundials_nvecserial.dylib)

add_executable(DummyResid DummyResid.cxx ComputeNode.cxx)
target_link_libraries(DummyResid
  CypressSim
  CypressCore)
