#ifndef CYPRESS_CORE_UTILITY
#define CYPRESS_CORE_UTILITY

#include <cmath>

namespace cypress
{

static inline
double bound_u(double v, double b)
{
  return std::max(v, b);
}

static inline
double bound_au(double v, double b)
{
  return (v/std::abs(v)) * std::max(std::abs(v), std::abs(b));
}

static inline
double bound_l(double v, double b)
{
  return std::min(v, b);
}

static inline
double bound_al(double v, double b)
{
  return (v/std::abs(v)) * std::min(std::abs(v), std::abs(b));
}

}

#endif
