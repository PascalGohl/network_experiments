/*
 * shared_memory.hpp
 *
 *  Created on: Jun 17, 2013
 *      Author: pascal
 */

#ifndef SHARED_MEMORY_MANAGER_HPP_
#define SHARED_MEMORY_MANAGER_HPP_

#include <vector>

#include "SharedRingBuffer.hpp"

class SharedMemoryManager {
 public:
  SharedMemoryManager();
  virtual ~SharedMemoryManager();

  int getMemoryAddress();
  void addRingBuffer(SharedRingBuffer buffer);
  SharedRingBuffer& getRingBuffer(int id);
  void printMemoryLayout();

 private:
  int allocated_memory_size_;
  int fpga_device;
  char * mmap_start_address_;
  std::vector<SharedRingBuffer> buffers_;
};

#endif /* SHARED_MEMORY_MANAGER_HPP_ */
