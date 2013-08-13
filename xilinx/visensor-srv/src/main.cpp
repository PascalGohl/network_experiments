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
#include "FPGA_config.hpp"
#include <time.h>

//#include "i2c-dev.h"
//#include "i2cbusses.h"

#include "MT9V034.hpp"

//#define IMAGE_SIZE 100 // debug value
#define IMAGE_SIZE 360960 //=752*480

#define IMAGE_FREQUENCY 60
#define IMU_FREQUENCY 20

#define TCP_DATA_PORT 13777
#define TCP_CONFIG_PORT 13778

void cam_faker(const boost::system::error_code& /*e*/,
		boost::asio::deadline_timer* t,
		SharedMemoryManager* shared_memory)
{
//	shared_memory->cam(0).setNewDataAvailable(true);
//	shared_memory->cam(1).setNewDataAvailable(true);

	// restart the timer
	t->expires_at(t->expires_at() + boost::posix_time::milliseconds(33));
	t->async_wait(boost::bind(cam_faker,
			boost::asio::placeholders::error, t, shared_memory));
}


void imu_faker(const boost::system::error_code& /*e*/,
		boost::asio::deadline_timer* t,
		SharedMemoryManager* shared_memory)
{
//	shared_memory->imu().setNewDataAvailable(true);

	// restart the timer
	t->expires_at(t->expires_at() + boost::posix_time::milliseconds(50));
	t->async_wait(boost::bind(imu_faker,
			boost::asio::placeholders::error, t, shared_memory));
}


int main(void)
{
	//	int i2cDescriptor;
	//	std::string i2cFileName = "/dev/i2c_1";
	//	i2cDescriptor = open_i2c_dev(1, (char*)i2cFileName.c_str(), 0, 0);
	//
	//	if (i2cDescriptor == -1) {
	//		throw printf("Failed to open I2C device.");
	//		exit(-1);
	//	}
	//
	//    set_slave_addr(i2cDescriptor, 0x48, 0);
	//
	//
	//	i2c_smbus_write_word_data(i2cDescriptor, 0xB2, 0x0000);
	//
	//
	//	close(i2cDescriptor);
	//	exit(0); // for faster debug




	//	DevMem mem(0x41200000, 4);
	//
	//	char val = 0;
	//	val &= ~(1 << 0);
	//	val |= 1 << 1;
	//	mem.write_byte(0, val);
	//	struct timespec slptm;
	//	slptm.tv_sec = 0;
	//	slptm.tv_nsec = 100000000;
	//
	//	bool toggle = false;
	//	for(int i = 0; i < 10; i++)
	//	{
	//		if(toggle)
	//		{
	//			val &= ~(1 << 0);
	//			val |= 1 << 1;
	//			toggle = false;
	//		}
	//		else{
	//			val &= ~(1 << 1);
	//			val |= 1 << 0;
	//			toggle = true;
	//		}
	//		mem.write_byte(0, val);
	//
	//		nanosleep(&slptm,NULL);
	//	}
	//
	//	val = mem.read_byte(0);
	//	val &= ~(1 << 0);
	//	val &= ~(1 << 0);
	//	mem.write_byte(0, val);
	//	exit(0); // for faster debug

	// init shared memory
	SharedMemoryManager shared_memory;
	shared_memory.addRingBuffer(SharedRingBuffer(IMAGE_SIZE, 3));
	shared_memory.printMemoryLayout();


	FPGAConfig fpga_config;
	fpga_config.set_data_buffer_address(shared_memory.getMemoryAddress());
	fpga_config.set_data_buffer_size(IMAGE_SIZE*3);
	//	fpga_config.set_num_cams(1);
	fpga_config.print_config();

	MT9V034 cam0;
	cam0.power_on();
		exit(0); // for faster debug

	//	char * data = shared_memory.cam(0).data();
	//	printf("Adress: 0x%x\n", data);
	//	for(int i = 0; i<40; i+=4)
	//	{
	//	  unsigned int * word = (unsigned int *)(data + i);
	//    // Dereference and convert it.
	//	  printf("%x ", (*word));
	//	}
	//	printf("\n");
	//	exit(0);

	try
	{
		boost::asio::io_service io_service;
		TcpServer tcp_server(io_service, TCP_DATA_PORT);
		TcpServer config_server(io_service, TCP_CONFIG_PORT);
		//		UdpServer udp_server(io_service);

		//fake the fpga for debug purpose
		boost::asio::deadline_timer t1(io_service, boost::posix_time::milliseconds(1000/IMAGE_FREQUENCY));
		//		t1.async_wait(boost::bind(cam_faker,
		//				boost::asio::placeholders::error, &t1, &shared_memory));
		//		boost::asio::deadline_timer t2(io_service, boost::posix_time::milliseconds(1000/IMU_FREQUENCY));
		//		t2.async_wait(boost::bind(imu_faker,
		//				boost::asio::placeholders::error, &t2, &shared_memory));


		// run servers in other thread
		boost::asio::io_service::work work(io_service);
		boost::thread bt(boost::bind(&boost::asio::io_service::run, &io_service));
		//		io_service.run();

		//		    std::vector<boost::asio::const_buffer> buffers;
		//		    buffers.push_back( boost::asio::buffer(&header, sizeof(header)) );
		//		    buffers.push_back( buf.data() );

		// send out images if client connected
		while(1){

			// check if
			if(tcp_server.has_connection() == false)
			{
				t1.wait();
				t1.expires_from_now(boost::posix_time::milliseconds(1000/IMAGE_FREQUENCY));
				continue;
			}

			//			boost::asio::ip::udp::endpoint udp_endpoint(
			//					tcp_server.getEndpoint().address(), 13778);

			for(int i = 0; i < 1; i++)
			{

				//				if(shared_memory.cam(i).newDataAvailable())
				//				{
				//					udp_server.send_data(i, shared_memory.cam(0), udp_endpoint);
				//					shared_memory.cam(i).setNewDataAvailable(false);
				//				}
				//
				//				if(shared_memory.imu().newDataAvailable())
				//				{
				//					tcp_server.send_data(shared_memory.imu());
				//					shared_memory.imu().setNewDataAvailable(false);
				//				}
				//
				//				if(shared_memory.config().newDataAvailable())
				//				{
				//					tcp_server.send_data(shared_memory.config());
				//					shared_memory.config().setNewDataAvailable(false);
				//				}
			}

			//			printf("send images\n");
			for(int i = 0; i<1; i++)
			{
				DataHeader header_cam;
				header_cam.timestamp = 0;
				header_cam.data_size = shared_memory.getRingBuffer(0).size();
				header_cam.data_id = 100+i;
				tcp_server.send_data(shared_memory.getRingBuffer(0).data(), header_cam);
			}


			t1.wait();
			t1.expires_from_now(boost::posix_time::milliseconds(1000/IMAGE_FREQUENCY));
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	return 0;
}
