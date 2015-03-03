#ifndef CYPRESS_COMPILE_DIAGNOSTICS_HXX
#define CYPRESS_COMPILE_DIAGNOSTICS_HXX

#include <iostream>
#include <string>
#include <vector>

namespace cypress { namespace compile {

struct Diagnostic
{
  enum class Level : int { Error=0, Warning=1, Info=2 };
  Level level{Level::Info};
  std::string message;
  size_t line{0};
  Diagnostic(Level level, std::string message, size_t line)
    : level{level}, message{message}, line{line} 
  {}
};

struct DiagnosticReport
{
  std::vector<Diagnostic> diagnostics;
  Diagnostic::Level level{Diagnostic::Level::Info};
  bool catastrophic();
};
 
std::ostream& operator<<(std::ostream &, const Diagnostic &);
std::ostream& operator<<(std::ostream &, const DiagnosticReport &);

struct CompilationError : public std::exception
{
  DiagnosticReport report;

  CompilationError(DiagnosticReport dr) : report(dr) {}

  const char* what() const noexcept override;
  
  private:
   mutable std::string what_;
};

}} //cypress::compile


#endif
