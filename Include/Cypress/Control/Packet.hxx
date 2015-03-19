#ifndef CYPRESS_CONTROL_CPACKET
#define CYPRESS_CONTROL_CPACKET

#include <array>
#include <iostream>
#include <string>

#if defined(__linux__)
#include <endian.h>
#elif defined(__APPLE__)
#include <libkern/OSByteOrder.h>
#define htobe64(x) OSSwapHostToBigInt64(x)
#define be64toh(x) OSSwapBigToHostInt64(x)
#endif

namespace cypress { namespace control {

struct CPacket
{
  std::array<char,4> hdr{'c', 'y', 'p', 'r'};
  unsigned long dst, sec, usec;
  double value;

  CPacket() = default;
  CPacket(unsigned long dst,
          unsigned long sec, unsigned long usec,
          double value)
    : dst{dst}, sec{sec}, usec{usec}, value{value}
  {}

  static CPacket fromBytes(char *buf);
  void toBytes(char *buf);
};

std::ostream& operator<<(std::ostream &o, const CPacket &c);

}} //::cypress::control

#endif
