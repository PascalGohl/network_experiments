/*
 * shared_memory.cpp
 *
 *  Created on: Jun 17, 2013
 *      Author: pascal
 */
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "shared_memory_manager.hpp"

/*
Shared memory layout:


----------------
1 Byte  | Image ready
----------------
n Bytes | Image Data
----------------
1 Byte  | Image ready
----------------
n Bytes | Image Data
----------------
1 Byte  | Image ready
----------------
n Bytes | Image Data
----------------
1 Byte  | Image ready
----------------
n Bytes | Image Data
----------------
1 Byte  |
 */

SharedMemoryManager::SharedMemoryManager(const int num_cams, const int image_size,
		const int imu_size, const int config_size)
: num_cams_(num_cams){

	// calculate needed space incl data available byte
	int mem_space = (image_size + 1) * num_cams_
			+ imu_size + 1
			+ config_size + 1;

	if ((fpga_device=open("/dev/slam-sensor", O_RDWR|O_SYNC))<0)
	{
		int myerr = errno;
		printf("ERROR: device open failed (errno %d %s)\n", myerr,
				strerror(myerr));
		exit(-1);
	}

	// allocate shared memory space
	char * addr = (char *) mmap(0, mem_space, PROT_READ|PROT_WRITE, MAP_SHARED, fpga_device, 0);
	if (addr == MAP_FAILED) {
		int myerr = errno;
		printf("ERROR: mmap failed (errno %d %s)\n", myerr,
				strerror(myerr));
		exit(-1);
	}

	// calculate data positions
	for(int i = 0; i<num_cams; i++)
		cams_.push_back(SharedMemory(addr + (image_size + HEADER_SIZE) * i, image_size));

	imu_ = SharedMemory(addr + (image_size + HEADER_SIZE) * num_cams, imu_size);
	config_ = SharedMemory(imu_.data() + imu_.size(), config_size);
}

SharedMemoryManager::~SharedMemoryManager() {
}

SharedMemory& SharedMemoryManager::cam(const int cam_id) {
	return cams_.at(cam_id);
}

SharedMemory& SharedMemoryManager::imu() {
	return imu_;
}

int SharedMemoryManager::getMemoryAddress() {
//	char * buffer = new char [5];
//	buffer[4] = '\0';
//	int ret = read(fpga_device, buffer, 4);
//	printf("ret: [%d], value: [%s]\n", ret, buffer);
//
//	for(int i = 0; i < 4; i++)
//		printf("%x", buffer[i]);
//	printf("\n");

	int add = 0;
	read(fpga_device, &add, 4);
//	printf("phys memory address: %x\n", add);
	return add;
}

SharedMemory& SharedMemoryManager::config() {
	return config_;
}
