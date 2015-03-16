#ifndef CYPRESS_CONTROL_CONTROLNODE
#define CYPRESS_CONTROL_CONTROLNODE

#include "Cypress/Core/Equation.hxx"
#include "Cypress/Core/Elements.hxx"

#include <ida/ida.h>
#include <ida/ida_dense.h>
#include <nvector/nvector_serial.h>
#include <sundials/sundials_math.h>
#include <sundials/sundials_types.h>

#include <string>
#include <vector>
#include <queue>
#include <unordered_map>
#include <sstream>
#include <memory>
#include <fstream>
#include <mutex>
#include <array>

#include <netinet/in.h>
#include <sys/socket.h>

#if defined(__linux__)
#include <endian.h>
#elif defined(__APPLE__)
#include <libkern/OSByteOrder.h>
#define htobe64(x) OSSwapHostToBigInt64(x)
#define be64toh(x) OSSwapBigToHostInt64(x)
#endif

namespace cypress { namespace control {

//fully qualified control variable
struct FQCV 
{ 
  FQCV(std::string who, std::string what) : who{who}, what{what} {}
  std::string who, what; 
};

//map for RX/TX from controllers
struct IOMap 
{ 
  IOMap(std::string local, FQCV remote) : local{local}, remote{remote} {}
  std::string local; FQCV remote; 
};

struct ControlNode
{
  ComponentSP source;
  std::string name;
  std::vector<IOMap> inputs, outputs;
  std::vector<EquationSP> eqtns;
  std::unordered_set<std::string> compute_vars;
  
  std::shared_ptr<std::stringstream> ss;

  ControlNode() 
    : ss{std::make_shared<std::stringstream>()} {}

  explicit ControlNode(ComponentSP source)
    : source{source}, name{source->name->value},
      ss{std::make_shared<std::stringstream>()} {}

  void extractComputeVars();
  void residualForm();
  void addInputResiduals();

  std::string emitSource() const;
  void emit_ctor() const;
  void emit_imapInit() const;
  void emit_resolveInit() const;
  void emit_accessors() const;
  void emit_residualFunc() const;
};

struct CPacket
{
  std::array<char,4> hdr{'c', 'y', 'p', 'r'};
  unsigned long who, what, sec, usec;
  double value;

  CPacket() = default;
  CPacket(
      unsigned long who, unsigned long what, 
      unsigned long sec, unsigned long usec,
      double value)
    : who{who}, what{what}, sec{sec}, usec{usec}, value{value}
  {}

  static CPacket fromBytes(char *buf);
  void toBytes(char *buf);
};

std::ostream& operator<<(std::ostream &o, const CPacket &c);

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
  //std::unordered_map<unsigned long, std::vector<CVal>> buf;
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
  std::unordered_map<unsigned long, CCoord> omap;

  //maps a local variable index to a resolver
  std::vector<FrameVarResolver> resolvers;
  
  ControlBuffer a_, b_;
  ControlBuffer *a{&a_}, *b{&b_};

  std::mutex io_mtx;

  void run();

  void rx(), tx();
  void listen();
  void send(CPacket pkt);
  void io();
  void kernel();
  void swapBuffers();
  void computeFrame();

  std::ofstream k_lg, io_lg;

  //Compute stuff
  size_t N;
  N_Vector nv_y, nv_dy;
  realtype *y, *dy, *c;

  //Comms stuff
  size_t port{4747};
  int sockfd;
  struct sockaddr_in servaddr, cliaddr;

  Controller(std::string name) 
    : name{name}, 
      k_lg{name+"k.log", std::ios_base::out | std::ios_base::app},
      io_lg{name+"io.log", std::ios_base::out | std::ios_base::app} 
  {}
  
  virtual void compute(realtype *r, realtype t) = 0;
};

std::ostream & operator << (std::ostream &, const ControlNode &);

}}

#endif
