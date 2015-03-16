#include "Cypress/Control/ControlNode.hxx"
#include <thread>
#include <chrono>
#include <csignal>
#include <stdexcept>
#include <string>

using std::string;
using namespace cypress;
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

  size_t at = 0;
  who = ntohl(*reinterpret_cast<unsigned long*>(buf));
  at += sizeof(unsigned long);
  what = ntohl(*reinterpret_cast<unsigned long*>(buf+at));
  at += sizeof(unsigned long);
  sec = ntohl(*reinterpret_cast<unsigned long*>(buf+at));
  at += sizeof(unsigned long);
  usec = ntohl(*reinterpret_cast<unsigned long*>(buf+at));
  at += sizeof(unsigned long);
  value = *reinterpret_cast<double*>(buf+at);

  return CPacket{who, what, sec, usec, value};
}

void CPacket::toBytes(char *bytes)
{
  size_t at = 0;

  *reinterpret_cast<unsigned long*>(bytes) = htonl(who);
  at += sizeof(unsigned long);
  *reinterpret_cast<unsigned long*>(bytes+at) = htonl(what);
  at += sizeof(unsigned long);
  *reinterpret_cast<unsigned long*>(bytes+at) = htonl(sec);
  at += sizeof(unsigned long);
  *reinterpret_cast<unsigned long*>(bytes+at) = htonl(usec);
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

    CPacket pkt = CPacket::fromBytes(msg);
    //io_lg << log("msg: " + string(msg)) << endl;
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
