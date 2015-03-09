add_executable(RotorParserTests RotorParserTests.cxx)

target_link_libraries(RotorParserTests
  gtest 
  gtest_main
  CypressC
  CypressCore
)

add_executable(RotorLinkTests RotorLinkTests.cxx)

target_link_libraries(RotorLinkTests
  gtest 
  gtest_main
  CypressC
  CypressCore
)
