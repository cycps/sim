#include <Cypress/Compiler/Driver.hxx>

#include <boost/exception/info.hpp>
#include <iostream>

using namespace cypress::compile;
namespace po = boost::program_options;
using std::cout;
using std::endl;

Driver::Driver(int argc, char **argv)
{
  buildInvocationOptionDescriptions();
  this->argc = argc;
  this->argv = argv;
}
  
void Driver::buildInvocationOptionDescriptions()
{
  opt_desc.add_options()
    ("version", "Show the Cypress compiler version")
    ("help", "Show this menu");
}

void Driver::showHelp()
{
  showVersion();
  cout << opt_desc << endl;
}

void Driver::showVersion()
{
  cout << "Cypress Compiler -- " 
    << COMPILER_VERSION_MAJOR
    << "."
    << COMPILER_VERSION_MINOR
    << endl;
}

void Driver::run()
{
  po::store(po::parse_command_line(argc, argv, opt_desc), opt_vmap);
  if(opt_vmap.count("help")) showHelp();
  else if(opt_vmap.count("version")) showVersion();
  else showHelp();

}
