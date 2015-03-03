add_executable(RotorParserTests RotorParserTests.cxx)

target_link_libraries(RotorParserTests
  gtest 
  gtest_main
  CypressC
  CypressCore
)
