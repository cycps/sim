#include "Cypress/Control/ControlNode.hxx"
#include "Cypress/Core/Elements.hxx"
#include <thread>
#include <chrono>
#include <csignal>
#include <stdexcept>
#include <string>

using std::string;
using namespace cypress::control;
using std::endl;
using std::this_thread::sleep_for;
using std::chrono::milliseconds;
using std::lock_guard;
using std::mutex;
using std::thread;
using std::runtime_error;
using std::to_string;
using std::ostream;
using std::make_shared;

void ControlNode::extractComputeVars()
{
  VarExtractor extractor(
      [](SymbolSP, VarContext)
      {
        //return !ctx.input;
        return true;
      });

  for(EquationSP eq: eqtns)
    extractor.run(source, eq);

  for(VarRefSP v : extractor.vars)
    compute_vars.insert(v->name);
}

void ControlNode::residualForm()
{
  for(EquationSP eq: eqtns)
  {
    setToZero(eq);
  }
}

void ControlNode::addInputResiduals()
{
  static constexpr int nosrc{-1};
  for(const IOMap &iom: inputs)
  {
    EquationSP eq = make_shared<Equation>(nosrc, nosrc);
    eq->lhs = make_shared<Real>(0, nosrc, nosrc);
    IOVarSP iov = 
          make_shared<IOVar>(make_shared<Symbol>(iom.local, nosrc, nosrc));

    iov->iokind = IOVar::IOKind::Input;

    eq->rhs =
      make_shared<Subtract>(
          make_shared<Symbol>(iom.local, nosrc, nosrc), iov,
          nosrc, nosrc);

    eqtns.push_back(eq);
  }
}

void Controller::listen()
{
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if(sockfd < 0)
    throw runtime_error{"socket() failed: " + to_string(sockfd)};

  io_lg << log("got socket " + to_string(sockfd)) << endl;

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(port);

  auto *addr = reinterpret_cast<const struct sockaddr*>(&servaddr);
  int err = bind(sockfd, addr, sizeof(servaddr));
  if(err < 0)
    throw runtime_error{"bind() failed: " + to_string(err)};

  io_lg << log("bound to port " + to_string(port)) << endl;

  io_lg << log("Listening") << endl;

  io();
}

void Controller::send(CPacket)
{
  //TODO
  io_lg << log("Sending") << endl;
}

void Controller::swapBuffers()
{
  lock_guard<mutex> lk(io_mtx);
}

void Controller::computeFrame()
{

}

void Controller::tx()
{

}

void Controller::kernel()
{
  k_lg << log("Kernel started") << endl;
  while(true)
  {
    swapBuffers();
    computeFrame();
    tx();
    //k_lg << log(".") << endl;
    sleep_for(milliseconds(period));
  }
}

CPacket CPacket::fromBytes(char *buf)
{
  unsigned long who, what, sec, usec;
  double value;

  char head[5];
  /*
  head[0] = buf[0];
  head[1] = buf[1];
  head[2] = buf[2];
  head[3] = buf[3];
  */
  strncpy(head, buf, 4);
  head[4] = 0;

  if(strncmp("cypr", head, 4) != 0)
  {
    throw runtime_error("Bad packet header `" + string(head) + "`");
  }

  size_t at = 4;
  who = be64toh(*reinterpret_cast<unsigned long*>(buf+at));
  at += sizeof(unsigned long);
  what = be64toh(*reinterpret_cast<unsigned long*>(buf+at));
  at += sizeof(unsigned long);
  sec = be64toh(*reinterpret_cast<unsigned long*>(buf+at));
  at += sizeof(unsigned long);
  usec = be64toh(*reinterpret_cast<unsigned long*>(buf+at));
  at += sizeof(unsigned long);
  value = *reinterpret_cast<double*>(buf+at);

  return CPacket{who, what, sec, usec, value};
}

void CPacket::toBytes(char *bytes)
{

  /*
  bytes[0] = 'c';
  bytes[1] = 'y';
  bytes[2] = 'p';
  bytes[3] = 'r';
  */

  strncpy(bytes, hdr.data(), 4);

  size_t at = 4;
  *reinterpret_cast<unsigned long*>(bytes+at) = htobe64(who);
  at += sizeof(unsigned long);
  *reinterpret_cast<unsigned long*>(bytes+at) = htobe64(what);
  at += sizeof(unsigned long);
  *reinterpret_cast<unsigned long*>(bytes+at) = htobe64(sec);
  at += sizeof(unsigned long);
  *reinterpret_cast<unsigned long*>(bytes+at) = htobe64(usec);
  at += sizeof(unsigned long);
  *reinterpret_cast<double*>(bytes+at) = value;

}

void Controller::io()
{
  socklen_t len;
  constexpr size_t sz{sizeof(CPacket)};
  char msg[sz];
  auto *addr = reinterpret_cast<struct sockaddr*>(&cliaddr);

  io_lg << log("entering io loop") << endl;
  while(true)
  {
    //BLOCKING!!
    int err = recvfrom(sockfd, msg, sz, 0, addr, &len);
    if(err < 0)
      throw runtime_error{"recvfrom() failed: " + to_string(err)};

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

    io_lg << ts() << pkt << endl;
    
    lock_guard<mutex> lk(io_mtx);
    b->add(pkt);
  }
}

void Controller::run()
{
  k_lg << log("up") << endl;

  thread t_io([this](){listen();});
  thread t_k([this](){kernel();});

  t_io.join();
  t_k.join();

  k_lg << log("down") << endl;

}

void ControlBuffer::add(CPacket pkt)
{
  buf[pkt.who+pkt.what].push_back({pkt.sec, pkt.usec, pkt.value});  
}

ostream& cypress::control::operator<<(ostream &o, const CPacket &c)
{
  o << "{" 
    << c.who << ", " 
    << c.what << ", " 
    << c.sec << ", " 
    << c.usec << ", " 
    << c.value 
    << "}";
  return o;
}

//Resolvers -------------------------------------------------------------------

double cypress::control::UseLatestArrival(const std::vector<double> &v)
{
  return v.back();
}
