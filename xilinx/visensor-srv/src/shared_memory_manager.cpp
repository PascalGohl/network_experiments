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
#include <boost/foreach.hpp>

#include "shared_memory_manager.hpp"

/*
Shared memory layout:

----------------
4 Byte  | Image ready
----------------
n Bytes | Image Data
----------------
4 Byte  | Image ready
----------------
n Bytes | Image Data
----------------
4 Byte  | Image ready
----------------
n Bytes | Image Data
----------------
4 Byte  | Image ready
----------------
n Bytes | Image Data
----------------
1 Byte  |
 */
//
//SharedMemoryManager::SharedMemoryManager(const int num_cams, const int image_size,
//		const int imu_size, const int config_size)
//: num_cams_(num_cams){
//
//	// calculate needed space incl data available byte
//	int mem_space = (image_size + HEADER_SIZE) * num_cams_
//			+ imu_size + HEADER_SIZE
//			+ config_size + HEADER_SIZE;
//
//	if ((fpga_device=open("/dev/slam-sensor", O_RDWR|O_SYNC))<0)
//	{
//		int myerr = errno;
//		printf("ERROR: device open failed (errno %d %s)\n", myerr,
//				strerror(myerr));
//		exit(-1);
//	}
//
//	// allocate shared memory space
//	char * addr = (char *) mmap(0, mem_space, PROT_READ|PROT_WRITE, MAP_SHARED, fpga_device, 0);
////	printf("mmap address: 0x%x\n", addr);
//	if (addr == MAP_FAILED) {
//		int myerr = errno;
//		printf("ERROR: mmap failed (errno %d %s)\n", myerr,
//				strerror(myerr));
//		exit(-1);
//	}
//
//	// calculate data positions
//	for(int i = 0; i<num_cams; i++)
//		cams_.push_back(SharedRingBuffer(addr + (image_size + HEADER_SIZE) * i, image_size));
//
//	imu_ = SharedRingBuffer(addr + (image_size + HEADER_SIZE) * num_cams, imu_size);
//	config_ = SharedRingBuffer(imu_.data() + imu_.size(), config_size);
//}

SharedMemoryManager::SharedMemoryManager(){
  if ((fpga_device=open("/dev/slam-sensor", O_RDWR|O_SYNC))<0)
  {
    int myerr = errno;
    printf("ERROR: device open failed (errno %d %s)\n", myerr,
           strerror(myerr));
    exit(-1);
  }


  // get the size of the allocated memory
  read(fpga_device, &allocated_memory_size_, 1);
  printf("Max memory space: %d\n", allocated_memory_size_);

  // allocate shared memory space
  mmap_start_address_ = (char *) mmap(0, allocated_memory_size_, PROT_READ|PROT_WRITE, MAP_SHARED, fpga_device, 0);
  //  printf("mmap address: 0x%x\n", addr);
  if (mmap_start_address_ == MAP_FAILED) {
    int myerr = errno;
    printf("ERROR: mmap failed (errno %d %s)\n", myerr,
           strerror(myerr));
    exit(-1);
  }

}

SharedMemoryManager::~SharedMemoryManager() {
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
  read(fpga_device, &add, 0);
  //	printf("phys memory address: %x\n", add);
  return add;
}

void SharedMemoryManager::addRingBuffer(SharedRingBuffer buffer) {
  char * buffer_start_address = mmap_start_address_;
  BOOST_FOREACH( SharedRingBuffer buf, buffers_ )
  {
    buffer_start_address += buf.size();
  }

  // check if new buffer fits into memory space
  if((int)buffer_start_address + buffer.size()*buffer.length() > allocated_memory_size_)
  {
    printf("ERROR: Desired buffer layout is longer than allocated memory");
    return;
  }
  buffer.setAddr(buffer_start_address);
  buffers_.push_back(buffer);
}

SharedRingBuffer& SharedMemoryManager::getRingBuffer(int id) {
  return buffers_.at(id);
}

void SharedMemoryManager::printMemoryLayout() {
  printf("==============================\n");
  BOOST_FOREACH( SharedRingBuffer buf, buffers_ )
  {
    printf("start address: 0x%04X\n", buf.data());
    printf("length: %d\n", buf.length());
  }
  printf("==============================\n");
}
