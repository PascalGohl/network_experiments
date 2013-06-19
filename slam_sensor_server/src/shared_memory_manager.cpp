/*
 * shared_memory.cpp
 *
 *  Created on: Jun 17, 2013
 *      Author: pascal
 */
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <errno.h>
#include "shared_memory_manager.hpp"

SharedMemoryManager::SharedMemoryManager(const int num_cams, const int image_size,
                           const int imu_size, const int config_size)
: num_cams_(num_cams){

  // calculate needed space incl data available byte
  int mem_space = (image_size + 1) * num_cams_
      + imu_size + 1
      + config_size + 1;

  // allocate shared memory space
  char * addr = (char *) mmap(0, mem_space, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  if (addr == 0) {
    int myerr = errno;
    printf("ERROR: mmap failed (errno %d %s)\n", myerr,
           strerror(myerr));
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

SharedMemory& SharedMemoryManager::config() {
  return config_;
}
