#include "Cypress/Compiler/Driver.hxx"
#include "Cypress/Compiler/Sema.hxx"
#include "Cypress/Core/Equation.hxx"

#include <boost/exception/info.hpp>
#include <boost/filesystem/convenience.hpp>
#include <iostream>
#include <fstream>
#include <streambuf>
#include <ostream>
#include <cstdlib>
#include <stdexcept>
#include <sys/stat.h>
#include <string>

using namespace cypress::compile;
using namespace cypress::sim;
namespace po = boost::program_options;
using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::ifstream;
using std::ofstream;
using std::getenv;
using std::runtime_error;
using std::to_string;
using std::make_shared;

Driver::Driver(int argc, char **argv)
{
  buildInvocationOptionDescriptions();
  this->argc = argc;
  this->argv = argv;
  decls = make_shared<Decls>();
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

void Driver::init()
{
  po::store(po::command_line_parser(argc, argv)
                  .options(opt_desc)
                  .positional(popt_desc).run(), 
            opt_vmap);
  po::notify(opt_vmap);
}

void Driver::run()
{
  init();

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
    currentInput = inf;
    string src = readSource(inf);
    compileSource(src);
  }
}

void Driver::parseInput()
{
  auto input_files = opt_vmap["input-file"].as<vector<string>>();
  for(const auto& inf : input_files)
  {
    currentInput = inf;
    string src = readSource(inf);
    parseSource(src);
  }
}

void Driver::checkSemantics()
{
  vector<ElementSP> elems;

  elems.insert(elems.end(), 
      decls->objects.begin(), 
      decls->objects.end());

  elems.insert(elems.end(), 
      decls->controllers.begin(), 
      decls->controllers.end());

  for(auto exp : decls->experiments)
  {
    DiagnosticReport dr = check(exp, elems);
    
    if(!dr.diagnostics.empty())
      cout << dr << endl;
    
    if(dr.catastrophic()) throw CompilationError(dr);
  }
}

void Driver::applyBounds()
{
  vector<ElementSP> elems;

  elems.insert(elems.end(), 
      decls->objects.begin(), 
      decls->objects.end());

  elems.insert(elems.end(), 
      decls->controllers.begin(), 
      decls->controllers.end());

  //VarLifter<BoundedVar> vl;
  for(ElementSP e : elems)
  {
    for(BoundSP b : e->bounds)
    {
      VarLifter<BoundVar> vl(b->lhs->varname());
      if(b->lhs->kind() == Expression::Kind::Differentiate)
        vl.lifts_vars = false,
        vl.lifts_derivs = true;

      for(EquationSP eq: e->eqtns) eq->accept(vl);
    }
  }

}

void Driver::parseSource(const std::string src)
{
  Parser p(src, dr);
  *decls += *p.run();
}

void Driver::buildSim(size_t N)
{
  sim = make_shared<Sim>(decls->objects, decls->experiments[0]); 
  sim->buildPhysics();
  sim_ex = sim->buildSimEx(N);
}
    
void Driver::createCypk()
{
  boost::filesystem::path pkgdir(decls->experiments[0]->name->value+".cypk");
  boost::filesystem::create_directory(pkgdir);
  
  ofstream ofs;

  size_t ix{0};
  for(const string &s: sim_ex.computeNodeSources)
  {
    ofs.open(pkgdir.string() + "/" + "CNode" + to_string(ix++) + ".cxx");
    ofs << s;
    ofs.close();
  }

  string brs{pkgdir.string() + "/" + "build_rcomp.sh"};
  ofs.open(brs);
  char *cyh_ = getenv("CYPRESS_HOME");
  if(cyh_ == nullptr)
    throw runtime_error("CYPRESS_HOME environment variable must be set");

  string cyhome{cyh_};
  ofs << "#!/bin/sh" << endl;
  for(size_t i=0; i<sim_ex.computeNodeSources.size(); ++i)
  {
    ofs
      << "clang++ -std=c++11 " 
      << "Cnode"<<i<<".cxx "
      << cyhome << "/Source/Sim/ComputeNodeDirectMain.cxx "
      << "-I" << cyhome << "/Include "
      << "-I" << "/usr/local/include "
      << "-L" << "/usr/local/lib "
      << "-lmpi "
      << "-lsundials_ida "
      << "-lsundials_nvecserial "
      << "-o " << "rcomp" << i << endl;
  }
  ofs.close();
    
  chmod(brs.c_str(), strtol("0755", 0, 8));
}

void Driver::compileSource(const string &src)
{
  Parser p(src, dr);
  *decls += *p.run();
  vector<ElementSP> elems;

  elems.insert(elems.end(), 
      decls->objects.begin(), 
      decls->objects.end());

  elems.insert(elems.end(), 
      decls->controllers.begin(), 
      decls->controllers.end());
 
  for(auto exp : decls->experiments)
  {
    DiagnosticReport dr = check(exp, elems);
    if(!dr.diagnostics.empty())
      cout << dr << endl;

    if(dr.catastrophic()) exit(1);

    Sim sim(decls->objects, exp); 
    sim.buildPhysics();
    SimEx sx = sim.buildSimEx(1);

    boost::filesystem::path pkgdir(exp->name->value+".cypk");
    boost::filesystem::create_directory(pkgdir);
    
    ofstream ofs;

    size_t ix{0};
    for(const string &s: sx.computeNodeSources)
    {
      ofs.open(pkgdir.string() + "/" + "CNode" + to_string(ix++) + ".cxx");
      ofs << s;
      ofs.close();
    }

    string brs{pkgdir.string() + "/" + "build_rcomp.sh"};
    ofs.open(brs);
    char *cyh_ = getenv("CYPRESS_HOME");
    if(cyh_ == nullptr)
      throw runtime_error("CYPRESS_HOME environment variable must be set");

    string cyhome{cyh_};
    ofs << "#!/bin/sh" << endl;
    for(size_t i=0; i<sx.computeNodeSources.size(); ++i)
    {
      ofs
        << "clang++ -std=c++11 " 
        << "Cnode"<<i<<".cxx "
        << cyhome << "/Source/Sim/ComputeNodeMain.cxx "
        << "-I" << cyhome << "/Include "
        << "-I" << "/usr/local/include "
        << "-L" << "/usr/local/lib "
        << "-lmpi "
        << "-lsundials_ida "
        << "-lsundials_nvecparallel "
        << "-o " << "rcomp" << i << endl;
    }
    ofs.close();

    
    chmod(brs.c_str(), strtol("0755", 0, 8));

    cout << "psys: " << endl;
    EqtnPrinter eqp;
    for(auto p : sim.psys) eqp.run(p.second);
    for(auto eq_str : eqp.strings) cout << eq_str << endl;
  }
}

string Driver::readSource(string file)
{
  ifstream ifs(file);
  string source_text;

  ifs.seekg(0, std::ios::end);
  source_text.reserve(ifs.tellg());
  ifs.seekg(0, std::ios::beg);

  source_text.assign((std::istreambuf_iterator<char>(ifs)),
                      std::istreambuf_iterator<char>());

  return source_text;
}
