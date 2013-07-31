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
#include "MT9V034.hpp"
#include <time.h>

#include "i2c-dev.h"
#include "i2cbusses.h"
#include "DevMem.h"

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

	DevMem mem(0x41600000, 256);

	// reset
	mem.write_word(0x040, 0xa);
	// set slave address

	//	mem.write_word(0x108, 0x90);
	//	// write register address
	//	mem.write_word(0x108, 0xB2);
	//	// set control register (init sending)
	//	mem.write_word(0x100, 0x0D);
	//	// write upper register value
	//	mem.write_word(0x108, 0x00);
	//	// write lower register value
	//	mem.write_word(0x108, 0x00 | 1<<9);
	//	// set slave address
	//	mem.write_word(0x108, 0x90);
	//	// write register address
	//	mem.write_word(0x108, 0xB2);
	//	// set control register (init sending)
	//	mem.write_word(0x100, 0x0D);
	//	// write upper register value
	//	mem.write_word(0x108, 0x00);
	//	// write lower register value
	//	mem.write_word(0x108, 0x00);
	//

	if(mem.read_word(0x104) & (1 << 6))
		printf("rx empty\n");
	if(mem.read_word(0x104) & (1 << 7))
		printf("tx empty\n");

	//slave address
	mem.write_word(0x108, 0x90 | 1<<8);
	// write register address
	mem.write_word(0x108, 0x00);
	// set control register (init sending)
	mem.write_word(0x100, 0x0D);

	for(int i = 0; !(mem.read_word(0x104) & (1 << 7)) && i < 10; i++)
	{
		printf("1waiting for empty\n");
	}
	//slave address
	mem.write_word(0x108, 0x91 | 1<<8);

	for(int i = 0; !(mem.read_word(0x104) & (1 << 7)) && i < 10; i++)
	{
		printf("2waiting for empty\n");
	}
	// config for reading
	mem.write_word(0x108, 0x02 | 1<<9);

	for(int j = 0; j < 2; j++)
	{
		printf("rx empty: %x\n", (mem.read_word(0x104) & (1 << 6)));
		for(int i = 0; (mem.read_word(0x104) & (1 << 6)) && i < 10; i++)
		{
			printf("3waiting for empty\n");
		}
		long val = mem.read_word(0x10C);

		printf("read byte: %x\n", val);
	}

	exit(0);
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

	MT9V034 cam0;
	cam0.power_on();
	exit(0); // for faster debug

	// init shared memory
	SharedMemoryManager shared_memory(2, IMAGE_SIZE, 10, 10);

	FPGAConfig fpga_config;
	fpga_config.set_data_buffer_address(shared_memory.getMemoryAddress());
	fpga_config.set_data_buffer_size(724992);
	fpga_config.set_num_cams(2);
	fpga_config.print_config();


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
