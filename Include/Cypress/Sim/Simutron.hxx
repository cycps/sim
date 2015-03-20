#ifndef CYPRESS_SIM_RESIDUALCLOSURE
#define CYPRESS_SIM_RESIDUALCLOSURE

#include <ida/ida.h>
#include <ida/ida_dense.h>
#include <nvector/nvector_serial.h>
#include <sundials/sundials_math.h>
#include <sundials/sundials_types.h>

#include <vector>
#include <functional>
#include <string>
#include <iostream>
#include <thread>
#include <array>
#include <mutex>
#include <fstream>

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#if defined(__linux__)
#include <endian.h>
#elif defined(__APPLE__)
#include <libkern/OSByteOrder.h>
#define htobe64(x) OSSwapHostToBigInt64(x)
#define be64toh(x) OSSwapBigToHostInt64(x)
#endif

#include <Cypress/Sim/Resolve.hxx>
#include <Cypress/Control/Packet.hxx>
#include <Cypress/Sim/Sensor.hxx>

namespace cypress { namespace sim {

sockaddr_in sensorSA(std::string);

struct Simutron
{
  N_Vector nv_y, nv_dy, nv_r;
  realtype *y, *dy, *c, *r;
  realtype *ry, *rdy;
  MPI_Win ywin, dywin;
  MPI_Comm ycomm, dycomm;
  std::ofstream c_lg, io_lg, results;

  //Comms stuff ---------------------------------------------------------------
  unsigned short port{7474};
  int sockfd;
  struct sockaddr_in servaddr, cliaddr, mwaddr;
  std::thread *comm_thd{nullptr};

  SensorManager sensorManager{this};

  //maps hash(dst) to a local input index
  std::unordered_map<unsigned long, size_t> cmap;
  //maps local coord to hash(dst)
  std::unordered_map<size_t, size_t> outputs;

  std::mutex rx_mtx, tx_mtx;

  Simutron(std::string name);

  std::vector<DCoordinate> varmap;
  void startControlListener();
  void clistenSetup();
  void clisten();
  virtual void compute(realtype *r, realtype t) = 0;
  virtual void resolve() = 0;
  virtual void init() = 0;
  virtual std::string experimentInfo() = 0;
  virtual size_t id() = 0;
  virtual size_t L() = 0;
  virtual size_t N() = 0;
  virtual size_t cN() = 0;

};

}} //::cypress::sim

#endif
