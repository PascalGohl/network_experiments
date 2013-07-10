//============================================================================
// Name        : slam_sensor_server.cpp
// Author      : Pascal Gohl
// Version     :
// Copyright   : 
// Description :
//============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <boost/thread.hpp>
#include "tcp_server.hpp"
#include "udp_server.hpp"
#include "shared_memory_manager.hpp"

//#define IMAGE_SIZE 100 // debug value
#define IMAGE_SIZE 360960 //=752*480

#define IMAGE_FREQUENCY 30 //=752*480
#define IMU_FREQUENCY 20 //=752*480
void cam_faker(const boost::system::error_code& /*e*/,
           boost::asio::deadline_timer* t,
           SharedMemoryManager* shared_memory)
{
  shared_memory->cam(0).setNewDataAvailable(true);
  shared_memory->cam(1).setNewDataAvailable(true);

  // restart the timer
  t->expires_at(t->expires_at() + boost::posix_time::milliseconds(33));
  t->async_wait(boost::bind(cam_faker,
                            boost::asio::placeholders::error, t, shared_memory));
}


void imu_faker(const boost::system::error_code& /*e*/,
           boost::asio::deadline_timer* t,
           SharedMemoryManager* shared_memory)
{
  shared_memory->imu().setNewDataAvailable(true);

  // restart the timer
  t->expires_at(t->expires_at() + boost::posix_time::milliseconds(50));
  t->async_wait(boost::bind(imu_faker,
                            boost::asio::placeholders::error, t, shared_memory));
}


int main(void)
{

  // init shared memory
  SharedMemoryManager shared_memory(2, IMAGE_SIZE, 10, 10);

  char * data = shared_memory.cam(0).data();

  *((unsigned int *) data+0) = 0; //image counter at the beginning of the data

  //fill the data with numbers
  for(int i = 0; i<IMAGE_SIZE; i+=4)
  {
    char number [5];
    sprintf(((char *) data+i), "%d", i/4);
  }

  try
  {
    boost::asio::io_service io_service;
    TcpServer tcp_server(io_service);
    UdpServer udp_server(io_service);


    //fake the fpga for debug purpose
    boost::asio::deadline_timer t1(io_service, boost::posix_time::milliseconds(1000/IMAGE_FREQUENCY));
    t1.async_wait(boost::bind(cam_faker,
          boost::asio::placeholders::error, &t1, &shared_memory));
    boost::asio::deadline_timer t2(io_service, boost::posix_time::milliseconds(1000/IMU_FREQUENCY));
    t2.async_wait(boost::bind(imu_faker,
          boost::asio::placeholders::error, &t2, &shared_memory));


    // run servers in other thread
    boost::thread bt(boost::bind(&boost::asio::io_service::run, &io_service));
    //    io_service.run();


    // send out images if client connected
    while(1){

      // check if
      if(tcp_server.has_connection() == false)
      {
        continue;
      }

      boost::asio::ip::udp::endpoint udp_endpoint(
          tcp_server.getEndpoint().address(), 13778);

      for(int i = 0; i < shared_memory.getNumCams(); i++)
      {

        if(shared_memory.cam(i).newDataAvailable())
        {
          udp_server.send_data(i, shared_memory.cam(0), udp_endpoint);
          shared_memory.cam(i).setNewDataAvailable(false);
        }

        if(shared_memory.imu().newDataAvailable())
        {
          tcp_server.send_data(shared_memory.imu());
          shared_memory.imu().setNewDataAvailable(false);
        }

        if(shared_memory.config().newDataAvailable())
        {
          tcp_server.send_data(shared_memory.config());
          shared_memory.config().setNewDataAvailable(false);
        }
      }


    }
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
