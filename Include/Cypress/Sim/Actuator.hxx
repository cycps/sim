#ifndef CYPRESS_ACTUATOR
#define CYPRESS_ACTUATOR

namespace cypress { namespace sim {

struct Actuator
{
  double min{0}, max{0}, dmin{0}, dmax{0};
  Actuator() = default;
  Actuator(double min, double max, double dmin, double dmax)
    : min{min}, max{max}, dmin{dmin}, dmax{dmax}
  {}

  inline double clamp(double x, double p)
  {
    if(x < min) x = min;
    if(x > max) x = max;
    if((x-p) > dmax) x = p + dmax;
    if((x-p) < dmin) x = p + dmin;
    
    return x;
  }
};

}} //::cypress::sim

#endif
