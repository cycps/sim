#include "Cypress/Control/ControlNode.hxx"
#include <thread>
#include <chrono>

using std::string;
using namespace cypress;
using namespace cypress::control;
using std::endl;
using std::this_thread::sleep_for;
using std::chrono::milliseconds;
using std::lock_guard;
using std::mutex;
using std::thread;

void Controller::listen()
{
  io_lg << log("Listening") << endl;
}

void Controller::send(CPacket pkt)
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
    sleep_for(milliseconds(period));
    swapBuffers();
    computeFrame();
    tx();
  }
}

void Controller::io()
{
  CPacket pkt;

  //TODO: RecvFrom(...)
  
  lock_guard<mutex> lk(io_mtx);

  b->add(pkt);
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
  buf[pkt.who+pkt.what].push_back({pkt.t, pkt.value});  
}

