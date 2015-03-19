#ifndef CYPRESS_SIM_SENSOR
#define CYPRESS_SIM_SENSOR

#include <queue>
#include <limits>
#include <string>

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

namespace cypress { namespace sim {

struct Simutron;

struct Sensor
{
  size_t id, idx;
  double rate;
  double nxt;
  sockaddr_in out_addr;

  Sensor(size_t id, size_t idx, double rate, sockaddr_in out_addr)
    : id{id}, idx{idx}, rate{rate}, nxt{rate}, out_addr(out_addr)
  {}
};

struct SensorCmp
{
  bool operator()(const Sensor &a, const Sensor &b)
  {
    return a.nxt > b.nxt;
  }
};

struct SensorManager
{
  Simutron *sim;
  double t{0}, thresh{0.74e-3};

  explicit SensorManager(Simutron *s);

  void add(Sensor);
  void step(double t);
  void tx(Sensor);

  //Comms stuff
  int sockfd;
  struct sockaddr_in servaddr;

  private:
    std::priority_queue<Sensor, std::vector<Sensor>, SensorCmp> Q;
};

}} //::cypress::sim

#endif
