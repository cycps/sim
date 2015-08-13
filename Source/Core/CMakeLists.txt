add_library(CypressCore SHARED Equation.cxx ElementIO.cxx Elements.cxx Var.cxx)
install(TARGETS CypressCore LIBRARY DESTINATION /usr/local/lib)

