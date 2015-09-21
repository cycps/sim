add_library(CypressC SHARED Driver.cxx Parser.cxx Sema.cxx Diagnostics.cxx)
target_link_libraries(CypressC 
  ${Boost_PROGRAM_OPTIONS_LIBRARY} 
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_FILESYSTEM_LIBRARY}
  CypressCore CypressSim CypressControl)

add_executable(cyc main.cxx)

target_link_libraries(cyc CypressC pthread)

install(TARGETS CypressC cyc
        LIBRARY DESTINATION /usr/local/lib
        RUNTIME DESTINATION /usr/local/bin)
