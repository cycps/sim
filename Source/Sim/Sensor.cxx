#include <stdexcept>

#include "Cypress/Sim/Sensor.hxx"
#include "Cypress/Sim/Simutron.hxx"

using std::runtime_error;
using std::endl;

using namespace cypress::sim;
using cypress::control::CPacket;

SensorManager::SensorManager(Simutron *sim)
  : sim{sim}
{
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if(sockfd < 0)
  {
    sim->io_lg << ts() << "SensorManager socket failure: " << sockfd << endl;
    throw runtime_error("io failure");
  }
}

void SensorManager::add(Sensor s)
{
  s.nxt = t + (1.0/s.rate);
  Q.push(s);
}

void SensorManager::step(double t)
{
  this->t = t; 

  sim->io_lg << ts() << "[sm] t=" << t << endl;
  sim->io_lg << ts() << "[sm] q=" << Q.top().nxt << endl;

  
  while(Q.top().nxt <= (t-thresh))
  {
    Sensor s = Q.top();
    tx(s);
    s.nxt = t + (1.0/s.rate);
    Q.pop();
    Q.push(s);
  }
}

void SensorManager::tx(Sensor s)
{
  double v = sim->y[s.idx];
  unsigned long sec = std::floor(t);
  unsigned long usec = std::floor((t - sec)*1e6);

  CPacket cpk{s.id, sec, usec, v};

  constexpr size_t sz{sizeof(CPacket)};
  char buf[sz];
  cpk.toBytes(buf);
  auto *addr = reinterpret_cast<sockaddr*>(&s.out_addr);
  sendto(sockfd, buf, sz, 0, addr, sizeof(s.out_addr));

  char sadr[128];
  inet_ntop(AF_INET, &(s.out_addr.sin_port), sadr, 128);

  sim->io_lg << ts() 
    << sadr << ":" << s.out_addr.sin_port << "  " <<  v
    << endl;
}
