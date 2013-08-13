/*
 * FPGAConfig.cpp
 *
 *  Created on: Jul 10, 2013
 *      Author: pascal
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "FPGA_config.hpp"

// FPGA Registers:
#define FPGA_SOFTWARE_VERSION  0x0000  // a increment of 100 means new incompatible interface

// Hardware register start addresses
#define CAM0_REGISTER_OFFSET   0X0010
#define CAM1_REGISTER_OFFSET   0X0030
#define CAM2_REGISTER_OFFSET   0X0050
#define CAM3_REGISTER_OFFSET   0X0070

// Camera registers
#define CAM_CONTROL            0x0000   // | 0: ENABLE | 1:....
#define CAM_TRIGGER_FREQUENCY  0x0004
#define CAM_BUFFER_ADDRESS     0x0008
#define CAM_BUFFER_SIZE        0x000C
#define CAM_FPGA_POINTER       0x0010
#define CAM_LINUX_POINTER      0x0014
#define CAM_RESERVED1          0x0018
#define CAM_RESERVED2          0x001C

#define BUFFER_ADD_OFFSET 4
#define BUFFER_SIZE_OFFSET 0
#define NUM_CAMS_OFFSET 8

#define MAP_SIZE 128
#define MAP_MASK (MAP_SIZE - 1)

FPGAConfig::FPGAConfig(){
	dev_base = 0x7e200000;
	memfd = open("/dev/mem", O_RDWR | O_SYNC);
	if (memfd == -1) {
		printf("Can't open /dev/mem.\n");
		exit(0);
	}
//	printf("/dev/mem opened.\n");

	// Map one page of memory into user space such that the device is in that page, but it may not
	// be at the start of the page.

	mapped_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, memfd,
			dev_base & ~MAP_MASK);
	if (mapped_base == (void *) -1) {
		printf("Can't map the memory to user space.\n");
		exit(0);
	}

	// get the address of the device in user space which will be an offset from the base
	// that was mapped as memory is mapped at the start of a page

	mapped_dev_base = mapped_base + (dev_base & MAP_MASK);
}

FPGAConfig::~FPGAConfig() {
	// unmap the memory before exiting

	if(memfd != 0)
	{
		if (munmap(mapped_base, MAP_SIZE) == -1) {
			printf("Can't unmap memory from user space.\n");
			exit(0);
		}
	}
	close(memfd);
}

void FPGAConfig::print_config() {
	printf("==============================\n");
	printf("Current FPGA config:\n");
	printf("Data Buffer Address: 0x%08x\n", get_data_buffer_address());
	printf("Data Buffer Size: %d\n", get_data_buffer_size());
	printf("Number of Cameras: %d\n", get_num_cams());
	printf("==============================\n");
}

int FPGAConfig::get_data_buffer_address() {
	int val = *((volatile int *) (mapped_dev_base + BUFFER_ADD_OFFSET));
	return val;
}

void FPGAConfig::set_data_buffer_address(const int address) {
	*((volatile int *) (mapped_dev_base + BUFFER_ADD_OFFSET)) = address;
}

int FPGAConfig::get_data_buffer_size() {
	int val = *((volatile int *) (mapped_dev_base + BUFFER_SIZE_OFFSET));
	return val;
}

void FPGAConfig::set_data_buffer_size(const int size) {
	*((volatile int *) (mapped_dev_base + BUFFER_SIZE_OFFSET)) = size;
}

int FPGAConfig::get_num_cams() {
	int val = *((volatile int *) (mapped_dev_base + NUM_CAMS_OFFSET));
	return val;
}

void FPGAConfig::set_num_cams(const int num_cams) {
	*((volatile int *) (mapped_dev_base + NUM_CAMS_OFFSET)) = num_cams;
}
