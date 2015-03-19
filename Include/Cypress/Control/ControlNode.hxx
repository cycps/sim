#ifndef CYPRESS_CONTROL_CONTROLNODE
#define CYPRESS_CONTROL_CONTROLNODE

#include <string>
#include <vector>
#include <queue>
#include <unordered_map>
#include <set>
#include <sstream>
#include <memory>
#include <fstream>
#include <mutex>
#include <array>

#include <netinet/in.h>
#include <sys/socket.h>

#include "Cypress/Control/Packet.hxx"
#include "Cypress/Core/Common.hxx"

#if defined(__linux__)
#include <endian.h>
#elif defined(__APPLE__)
#include <libkern/OSByteOrder.h>
#define htobe64(x) OSSwapHostToBigInt64(x)
#define be64toh(x) OSSwapBigToHostInt64(x)
#endif

namespace cypress { namespace control {

struct CVal
{
  unsigned long sec, usec;
  double v;

  CVal() = default;
  CVal(unsigned long sec, unsigned long usec, double v) 
    : sec{sec}, usec{usec}, v{v} {};
};

struct ControlBuffer
{
  std::vector<std::vector<CVal>> buf;
  size_t size() { return buf.size(); }
};

//Control Coordinate
struct CCoord
{
  unsigned long who{}, what{};
  CCoord() = default;
  CCoord(unsigned long who, unsigned long what)
    : who{who}, what{what}
  {}
};

using FrameVarResolver = std::function<double(const std::vector<CVal>&)>;

double UseLatestArrival(const std::vector<CVal> &);

struct Controller
{
  size_t period{100}; //100 millisecond default period
  std::string name;

  //maps hash(who+what) to a local input index
  std::unordered_map<unsigned long, size_t> imap;

  std::vector<double> input_frame;

  //maps local input index to local control index
  std::vector<unsigned long> ic_map;

  //maps a local variable index to a CCord
  //std::unordered_map<unsigned long, CCoord> omap;

  //maps a local variable index to a resolver
  std::vector<FrameVarResolver> resolvers;
  
  ControlBuffer a_, b_;
  ControlBuffer *a{&a_}, *b{&b_};

  std::mutex io_mtx;

  void run();

  CPacket cpk;
  std::hash<std::string> hsh{};
  void setTarget(std::string what);

  void rx(), tx(double v);
  void listen();
  void send(CPacket pkt);
  void io();
  void kernel();
  void swapBuffers();
  void computeFrame();
  void stepIda();
  void initIda();

  std::ofstream k_lg, io_lg;

  //Comms stuff ---------------------------------------------------------------
  size_t port{4747};
  int sockfd;
  struct sockaddr_in servaddr, cliaddr, tgtaddr;

  Controller(std::string name) 
    : name{name}, 
      k_lg{name+"k.log", std::ios_base::out | std::ios_base::app},
      io_lg{name+"io.log", std::ios_base::out | std::ios_base::app} 
  {}
  
  virtual void compute() = 0;
};

}}

#endif
