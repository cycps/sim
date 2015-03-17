#include "Cypress/Control/ControlNode.hxx"
#include "Cypress/Core/Elements.hxx"
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
  
size_t ControlNode::computeIndex(std::string s) const
{
  auto it = compute_vars.find(s);
  if(it == compute_vars.end())
    throw runtime_error{"bad compute index key"};

  return std::distance(compute_vars.begin(), it);
}

size_t ControlNode::inputIndex(std::string s) const
{
  auto it = std::find_if(inputs.begin(), inputs.end(),
      [s](const IOMap &iom){ return iom.local == s; });
  
  if(it == inputs.end())
    throw runtime_error{"bad local index key"};

  return std::distance(inputs.begin(), it);
}

size_t ControlNode::outputIndex(std::string s) const
{
  auto it = std::find_if(outputs.begin(), outputs.end(),
      [s](const IOMap &iom){ return iom.local == s; });
  
  if(it == outputs.end())
    throw runtime_error{"bad local index key"};

  return std::distance(outputs.begin(), it);
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
  for(size_t i=0; i<a->size(); ++i)
  {
    input_frame[i] = resolvers[i](a->buf[i]);
  }
}

void Controller::tx()
{
  auto *addr = reinterpret_cast<struct sockaddr*>(&mwaddr);
  CPacket cpk;
  
  auto tp = high_resolution_clock::now();
  auto dur = tp.time_since_epoch();
  
  size_t sec = duration_cast<seconds>(dur).count();
  dur -= seconds(sec);
  size_t usec = duration_cast<microseconds>(dur).count();

  for(const auto &p: omap)
  {
    cpk.who = p.second.who;
    cpk.what = p.second.what;
    cpk.sec = sec;
    cpk.usec = usec;
    cpk.value = y[p.first];
  
    sendto(sockfd, &cpk, sizeof(cpk), 0, addr, sizeof(mwaddr));
  }
}

void Controller::kernel()
{
  k_lg << log("Kernel started") << endl;
  while(true)
  {
    swapBuffers();
    //TODO
    //computeFrame();
    //stepIda();
    //tx();
    //k_lg << log(".") << endl;
    sleep_for(milliseconds(period));
  }
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
    a->buf[imap[pkt.who+pkt.what]].push_back({pkt.sec, pkt.usec, pkt.value});  
  }
}

void Controller::stepIda()
{
  double tret{ida_now};
  double secs = period/1.0e3;
  int retval = IDASolve(ida_mem, ida_now+secs, &tret, nv_y, nv_dy, IDA_NORMAL);

  if(retval != IDA_SUCCESS)
  {
    k_lg << ts() << "IDASolve failed: " << retval << endl;
    throw runtime_error{"IDASolve failed"};
  }

  if(tret - (ida_now+secs) > atl)
  {
    k_lg << ts() << "IDASolve error target integration time not reached" 
         << endl;
    throw runtime_error{"IDASolve error target integration time not reached"};
  }

}

bool Controller::checkInitialConds(double tol)
{
  realtype *r = (realtype*)malloc(sizeof(realtype) * N);
  compute(r, 0);
  bool ok{true};

  k_lg << ts() << "Checking initial conditions" << endl;
  for(size_t i=0; i<N; ++i)
  {
    k_lg << ts() << "r["<<i<<"]: " << r[i] << endl;  
    if(std::abs(r[i]) > tol) ok = false;
  }
  if(ok)
    k_lg << ts() << "pass" << endl;
  else
    k_lg << ts() << "fail" << endl;
  
  return ok;

}

void Controller::initIda()
{
  k_lg << log("Initializing Ida") << endl;

  nv_y = N_VNew_Serial(N);
  nv_dy = N_VNew_Serial(N);

  y = NV_DATA_S(nv_y);
  dy = NV_DATA_S(nv_dy);

  ida_mem = IDACreate();
  if(ida_mem == nullptr)
  {
    k_lg << ts() << "IDACreate failed" << endl;
    throw runtime_error{"IDACreate failed"};
  }

  int retval = IDAInit(ida_mem, F, ida_start, nv_y, nv_dy);
  if(retval != IDA_SUCCESS)
  {
    k_lg << ts() << "IDAInit failed: " << retval << endl;
    throw runtime_error{"IDAInit failed"};
  }

  retval = IDASetUserData(ida_mem, this);
  if(retval != IDA_SUCCESS)
  {
    k_lg << ts() << "IDASetUserData failed: " << retval << endl;
    throw runtime_error{"IDASetUserData failed"};
  }

  retval = IDASStolerances(ida_mem, rtl, atl);
  if(retval != IDA_SUCCESS)
  {
    k_lg << ts() << "IDASStolerances failed: " << retval << endl;
    throw runtime_error{"IDASStolerances failed"};
  }

  retval = IDADense(ida_mem, N);
  if(retval != IDA_SUCCESS)
  {
    k_lg << ts() << "IDADense failed: " << retval << endl;
    throw runtime_error{"IDADense failed"};
  }

  bool init_ok = checkInitialConds(atl);
  if(!init_ok)
    throw runtime_error{"Initial conditions check failed"};

  //Initialize state-trajectory space
  for(size_t i=0; i<N; ++i) y[i] = dy[i] = 0;

  k_lg << log("Ida ready") << endl;
}

void Controller::run()
{
  k_lg << log("up") << endl;

  //initIda();

  thread t_io([this](){listen();});
  thread t_k([this](){kernel();});

  t_io.join();
  t_k.join();

  k_lg << log("down") << endl;

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

double cypress::control::UseLatestArrival(const std::vector<CVal> &v)
{
  return v.back().v;
}
