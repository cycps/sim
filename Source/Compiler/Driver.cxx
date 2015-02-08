#include "Cypress/Compiler/Driver.hxx"
#include "Cypress/Compiler/Sema.hxx"
#include "Cypress/Core/Eqtn.hxx"

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
    string inp = readSource(inf);
    Parser p(inp);
    auto decls = p.run();

    for(shared_ptr<Object> obj : decls->objects)
    {
      for(shared_ptr<Equation> eqtn : obj->eqtns) setToZero(eqtn);
    }
    
    for(shared_ptr<Object> obj : decls->objects)
    {
      VarCollector vc{obj};
      vc.run();
      //for(shared_ptr<Equation> eqtn : obj->eqtns) eqtn->accept(vc);

      cout << endl;
      cout << "vars:" << endl;
      for(auto v : vc.vars)
        cout << v->value << endl;
      cout << endl;

      cout << "derivs:" << endl;
      for(auto d : vc.derivs)
        cout << d->value << endl;

      cout << endl;
    }

    EqtnPrinter eqp;
    for(shared_ptr<Object> obj : decls->objects)
    {
      for(shared_ptr<Equation> eqtn : obj->eqtns) 
      {
        eqtn->accept(eqp);
        cout << endl;
      }
    }

  }
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
