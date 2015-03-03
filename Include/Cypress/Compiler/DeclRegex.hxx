#ifndef CYPRESS_COMPILE_DECLREGEX
#define CYPRESS_COMPILE_DECLREGEX

#include <vector>
#include <memory>
#include <string>
#include <iostream>
#include <regex>
#include <unordered_map>
#include <set>

namespace cypress { namespace compile {
  
static inline std::regex& objrx()
{
  static std::regex *rx = 
    new std::regex{"Object\\s+([a-zA-Z_][a-zA-Z0-9_]*)(\\(.*\\))"};
  return *rx;
}

static inline std::regex& contrx()
{
  static std::regex *rx = 
    new std::regex{"Controller\\s+([a-zA-Z_][a-zA-Z0-9_]*)(\\(.*\\))"};
  return *rx;
}

static inline std::regex& exprx()
{
  static std::regex *rx = 
    new std::regex{"Experiment\\s+([a-zA-Z_][a-zA-Z0-9_]*)"};
  return *rx;
}

static inline std::regex& comprx()
{
  static std::regex *rx = 
    new std::regex{
      "\\s\\s+([a-zA-Z_][a-zA-Z0-9_]*)\\s+([a-zA-Z_][a-zA-Z0-9_]*)(\\(.*\\))*"};
  return *rx;
}

static inline std::regex& lnkrx()
{
  static std::regex *rx = 
    new std::regex{"\\s\\s+[a-zα-ωΑ-ΩA-Z_][a-zα-ωΑ-ΩA-Z0-9_\\.]*\\s+>\\s+.*"};
  return *rx;
}

static inline std::regex& thingrx()
{
  static std::regex *rx = 
    new std::regex{"([a-zα-ωΑ-ΩA-Z_][a-zα-ωΑ-ΩA-Z0-9_]*)"};
  return *rx;
}

static inline std::regex& subthingrx()
{
  static std::regex *rx = 
    new std::regex{ "([a-zα-ωΑ-ΩA-Z_][a-zα-ωΑ-ΩA-Z0-9_]*)"
                    "\\.([a-zα-ωΑ-ΩA-Z_][a-zα-ωΑ-ΩA-Z0-9_]*)"};
  return *rx;
}

static inline std::regex& atodrx()
{
  static std::regex *rx = 
    new std::regex{"\\|([0-9]+\\.[0-9]+)\\|"};
  return *rx;
}

}}

#endif
