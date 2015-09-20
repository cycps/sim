add_library(CypressCore Equation.cxx ElementIO.cxx Elements.cxx Var.cxx)
install(TARGETS CypressCore ARCHIVE DESTINATION /usr/local/lib)

