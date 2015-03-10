add_library(CypressControl SHARED ControlSystem.cxx ControlNode.cxx)
target_link_libraries(CypressControl
  CypressCore
  /usr/local/lib/libsundials_ida.dylib 
  /usr/local/lib/libsundials_nvecserial.dylib)
