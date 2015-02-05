add_library(CypressC SHARED Driver.cxx Parser.cxx AST.cxx Sema.cxx)
target_link_libraries(CypressC ${Boost_PROGRAM_OPTIONS_LIBRARY})

add_executable(cyc main.cxx)
target_link_libraries(cyc CypressC)

