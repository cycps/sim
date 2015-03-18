#include <stdexcept>
#include <string>
#include <iostream>

#include "Cypress/Sim/Simutron.hxx"

using namespace cypress::sim;
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


//TODO: Proper logging

void Simutron::clistenSetup()
{
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if(sockfd < 0)
    throw runtime_error{"socket() failed: " + to_string(sockfd)};

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(port);

  auto *addr = reinterpret_cast<const struct sockaddr*>(&servaddr);
  int err = bind(sockfd, addr, sizeof(servaddr));
  if(err < 0)
    throw runtime_error{"bind() failed: " + to_string(err)};
}

void Simutron::clisten()
{
  socklen_t len;
  constexpr size_t sz{sizeof(CPacket)};
  char msg[sz];
  auto *addr = reinterpret_cast<struct sockaddr*>(&cliaddr);

  while(true)
  {
    //BLOCKING!!!
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
      cerr << "packet read error: " << ex.what() << endl;
      continue;
    }

    cout << pkt << endl;

    lock_guard<mutex> lk(io_mtx);
    c[cmap[pkt.who+pkt.what]] = pkt.value; //last monkey wins!
  }
}

void Simutron::startControlListener()
{
  comm_thd = new thread([this](){ clisten(); });
  comm_thd->detach();
}

ostream& cypress::sim::operator<<(ostream &o, const CPacket &c)
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

CPacket CPacket::fromBytes(char *buf)
{
  unsigned long who, what, sec, usec;
  double value;

  char head[5];
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
