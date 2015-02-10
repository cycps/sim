#include "Cypress/Compiler/Driver.hxx"
#include "Cypress/Compiler/Sema.hxx"
#include "Cypress/Core/Eqtn.hxx"
#include "Cypress/Core/Sim.hxx"

#include <boost/exception/info.hpp>
#include <iostream>
#include <fstream>
#include <streambuf>

using namespace cypress::compile;
namespace po = boost::program_options;
using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::shared_ptr;

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
    ("help", "Show this menu")
    ("input-file", po::value<vector<string>>(), "input-file");

  popt_desc.add("input-file", -1);
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
  po::store(po::command_line_parser(argc, argv)
                  .options(opt_desc)
                  .positional(popt_desc).run(), 
            opt_vmap);
  po::notify(opt_vmap);

  if(opt_vmap.count("help")) showHelp();
  else if(opt_vmap.count("version")) showVersion();
  else if(opt_vmap.count("input-file")) compileInputFiles();
  else showHelp();

}

void Driver::compileInputFiles()
{
  auto input_files = opt_vmap["input-file"].as<vector<string>>();

  for(const auto& inf : input_files)
  {
    cout << "Compiling '" << inf << "'" << endl;
    string src = readSource(inf);
    compileSource(src);
  }
}

void Driver::compileSource(const string &src)
{
  Parser p(src);
  auto decls = p.run();
  auto objects = decls->objects;
  auto controllers = decls->controllers;

  vector<shared_ptr<Element>> elements;
  elements.insert(elements.end(), objects.begin(), objects.end());
  elements.insert(elements.end(), controllers.begin(), controllers.end());

  VarCollector vc;
  EqtnPrinter eqp;
  for(auto e : elements) 
  {
    setEqtnsToZero(e);
    eqp.run(e, true);
    vc.run(e);
  }
  //for(auto eq_str : eqp.strings) cout << eq_str << endl;
  //vc.showVars();
  //vc.showDerivs();
 
  eqp.strings.clear();
  for(auto exp : decls->experiments)
  {
    Sim sim(objects, controllers, exp); 
    sim.buildPhysics();
    cout << "psys: " << endl;
    for(auto eqtn : sim.psys) eqp.run(eqtn);
  }
  for(auto eq_str : eqp.strings) cout << eq_str << endl;
  
  eqp.strings.clear();
  for(auto e : objects) eqp.run(e);
  for(auto eq_str : eqp.strings) cout << eq_str << endl;
}

string Driver::readSource(string file)
{
  std::ifstream ifs(file);
  std::string source_text;

  ifs.seekg(0, std::ios::end);
  source_text.reserve(ifs.tellg());
  ifs.seekg(0, std::ios::beg);

  source_text.assign((std::istreambuf_iterator<char>(ifs)),
                      std::istreambuf_iterator<char>());

  return source_text;
}
