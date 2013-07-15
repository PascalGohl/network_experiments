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

/*
FPGA Register layout:

0 Bytes         4 Bytes           8 Bytes           12 Bytes            16 Bytes
 ----------------------------------------------------------------------------
|   Data Buffer Mem. Addr.         |   Buffer Size     |  Number of Cameras  |
 ----------------------------------------------------------------------------
|   Camera Rate  |    IMU Rate     |
 ----------------------------------------------------------------------------
 */

#define MAP_SIZE 16
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
	printf("Current FPGA config:\n");
	printf("Data Buffer Address: 0x%08x\n", get_data_buffer_address());
	printf("Data Buffer Size: %d\n", get_data_buffer_size());
	printf("Number of Cameras: %d\n", get_num_cams());
}

int FPGAConfig::get_data_buffer_address() {
	int val = *((volatile int *) (mapped_dev_base));
	return val;
}

void FPGAConfig::set_data_buffer_address(const int address) {
	*((volatile int *) (mapped_dev_base)) = address;
}

int FPGAConfig::get_data_buffer_size() {
	int val = *((volatile int *) (mapped_dev_base + 4));
	return val;
}

void FPGAConfig::set_data_buffer_size(const int size) {
	*((volatile int *) (mapped_dev_base + 4)) = size;
}

int FPGAConfig::get_num_cams() {
	int val = *((volatile int *) (mapped_dev_base + 8));
	return val;
}

void FPGAConfig::set_num_cams(const int num_cams) {
	*((volatile int *) (mapped_dev_base + 8)) = num_cams;
}
