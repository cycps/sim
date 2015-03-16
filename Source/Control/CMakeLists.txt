add_library(CypressControl 
  SHARED 
  ControlSystem.cxx 
  ControlNode.cxx ControlNodeEmit.cxx)

target_link_libraries(CypressControl
  CypressCore
  /usr/local/lib/libsundials_ida.dylib 
  /usr/local/lib/libsundials_nvecserial.dylib)

add_executable(DummyController DummyController.cxx ControlMain.cxx)
target_link_libraries(DummyController CypressControl)

add_executable(CPacketLauncher CPacketLauncher.cxx)
target_link_libraries(CPacketLauncher CypressControl)
