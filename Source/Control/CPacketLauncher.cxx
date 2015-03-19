#include <Cypress/Control/ControlNode.hxx>

#include <iostream>
#include <chrono>
#include <stdexcept>

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

using std::cout;
using std::cerr;
using std::cin;
using std::endl;
using std::flush;
using std::string;
using std::runtime_error;

using namespace cypress::control;
using namespace std::chrono;

int main(int argc, char **argv)
{
  if(argc != 4)
  {
    cerr << "usage: CPacketLauncher <addr> <dest> <value>" << endl;
    exit(1);
  }

  std::hash<string> hsh;
  string s_dst{argv[2]}, 
         s_value{argv[3]};

  unsigned long dst{hsh(s_dst)};

  double value{stod(s_value)};

  auto tp = high_resolution_clock::now();
  auto dur = tp.time_since_epoch();
  
  size_t sec = duration_cast<seconds>(dur).count();
  dur -= seconds(sec);
  size_t usec = duration_cast<microseconds>(dur).count();

  CPacket cpk{dst, sec, usec, value};

  string nil;
  cout << "launching: " << cpk << endl;

  int sockfd;
  struct sockaddr_in servaddr;

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(4747);
  int err = inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

  if(err < 0)
    throw runtime_error("bad server address");

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);

  if(sockfd < 0)
    throw runtime_error("unable to create socket");

  constexpr size_t sz = sizeof(CPacket);
  char buf[sz];
  cpk.toBytes(buf);

  auto *addr = reinterpret_cast<struct sockaddr*>(&servaddr);
  sendto(sockfd, buf, sz, 0, addr, sizeof(servaddr));

}
