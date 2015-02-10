#ifndef CYPRESS_COMPILE_DRIVER_HXX
#define CYPRESS_COMPILE_DRIVER_HXX

//#include "Grammar.hxx"
#include "Parser.hxx"
#include <boost/program_options.hpp>
#include <string>

namespace cypress { namespace compile {

static constexpr unsigned short 
  COMPILER_VERSION_MAJOR{0},
  COMPILER_VERSION_MINOR{1};

class Driver
{
  public:
    Driver(int argc, char **argv);
    void showHelp();
    void showVersion();
    void run();

  private:
    void buildInvocationOptionDescriptions();
    void compileInputFiles();
    void compileSource(const std::string&);
    std::string readSource(std::string file);

    int argc;
    char **argv;
    boost::program_options::options_description opt_desc;
    boost::program_options::variables_map opt_vmap;
    boost::program_options::positional_options_description popt_desc;
};

}}

#endif
