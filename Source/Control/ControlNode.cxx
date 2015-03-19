#include "Cypress/Control/ControlNode.hxx"
#include <thread>
#include <chrono>
#include <csignal>
#include <stdexcept>
#include <string>
#include <iostream>

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
using namespace std::chrono;

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
  for(size_t i=0; i<a->size(); ++i)
  {
    input_frame[i] = resolvers[i](a->buf[i]);
  }
}

void Controller::tx(double v)
{
  auto *addr = reinterpret_cast<struct sockaddr*>(&tgtaddr);
  
  auto tp = high_resolution_clock::now();
  auto dur = tp.time_since_epoch();
  
  size_t sec = duration_cast<seconds>(dur).count();
  dur -= seconds(sec);
  size_t usec = duration_cast<microseconds>(dur).count();

  cpk.sec = sec;
  cpk.usec = usec;
  cpk.value = v;

  sendto(sockfd, &cpk, sizeof(cpk), 0, addr, sizeof(tgtaddr));
}

void Controller::setTarget(string dst)
{
  cpk.dst = hsh(dst);
}


void Controller::kernel()
{
  k_lg << log("Kernel started") << endl;
  while(true)
  {
    swapBuffers();
    computeFrame();
    compute();
    sleep_for(milliseconds(period));
  }
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
    a->buf[imap[pkt.dst]].push_back({pkt.sec, pkt.usec, pkt.value});  
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

/*
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
*/

//Resolvers -------------------------------------------------------------------

double cypress::control::UseLatestArrival(const std::vector<CVal> &v)
{
  if(v.empty()) return 0;

  return v.back().v;
}
