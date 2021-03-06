#include <stdexcept>
#include <string>
#include <iostream>

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "Cypress/Sim/Simutron.hxx"

using namespace cypress::sim;
using cypress::control::CPacket;
using std::runtime_error;
using std::thread;
using std::to_string;
using std::cerr;
using std::cout;
using std::ostream;
using std::endl;
using std::mutex;
using std::lock_guard;
using std::string;
using std::ofstream;


Simutron::Simutron(string name)
  : c_lg{name+".compute.log", std::ios_base::out | std::ios_base::app},
    io_lg{name+".io.log", std::ios_base::out | std::ios_base::app},
    results{name+".results"}
{}

void Simutron::clistenSetup()
{
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if(sockfd < 0)
  {
    io_lg << ts() << "socket() failed: " << sockfd << endl;
    throw runtime_error{"io failure"};
  }
  
  io_lg << ts() << "got socket " << sockfd << endl;

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(port);

  auto *addr = reinterpret_cast<const struct sockaddr*>(&servaddr);
  int err = bind(sockfd, addr, sizeof(servaddr));
  if(err < 0)
  {
    io_lg << ts() << "bind() failed: " << err << endl;
    throw runtime_error{"io failure"};
  }
  
  io_lg << ts() << "bound to port " << port << endl;
}

void Simutron::clisten()
{
  socklen_t len;
  constexpr size_t sz{sizeof(CPacket)};
  char msg[sz];
  auto *addr = reinterpret_cast<struct sockaddr*>(&cliaddr);

  io_lg << log("Listening") << endl;

  while(true)
  {
    //BLOCKING!!!
    int err = recvfrom(sockfd, msg, sz, 0, addr, &len);
    if(err < 0)
    {
      io_lg << ts() << "recvfrom() failed: " << err << endl;
      throw runtime_error{"io failure"};
    }

    CPacket pkt;
    try
    {
      pkt = CPacket::fromBytes(msg);
    }
    catch(runtime_error &ex)
    {
      io_lg << ts() << "packet read error: " << ex.what() << endl;
      continue;
    }


    lock_guard<mutex> lk(rx_mtx);

    Actuator atr;
    try{ atr = amap.at(pkt.dst); }
    catch(std::out_of_range &)
    { 
      io_lg << ts() << "Unkown packet destination: " << pkt.dst << endl;
      continue; 
    }

    double old = c[cmap[pkt.dst]];
    double v = pkt.value;
    
    double clamped = atr.clamp(v, old);
    io_lg << ts() 
      << "c[" << cmap[pkt.dst] << "] = " 
      << v << " |" << clamped << "|" 
      << endl;
      
    c[cmap[pkt.dst]] = clamped;
  }
}

void Simutron::startControlListener()
{
  clistenSetup();
  comm_thd = new thread([this](){ clisten(); });
  comm_thd->detach();
}

sockaddr_in cypress::sim::sensorSA(std::string s)
{
  sockaddr_in addr;

  bzero(&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(4747); //TODO kill hardcode
  int err = inet_pton(AF_INET, s.c_str(), &addr.sin_addr);

  if(err < 0)
    throw runtime_error{"Invalid sensor target address"};

  return addr;
}

