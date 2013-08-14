/*
 * shared_memory.hpp
 *
 *  Created on: Jun 17, 2013
 *      Author: pascal
 */

#ifndef SHARED_RING_BUFFER_HPP_
#define SHARED_RING_BUFFER_HPP_

#include "FPGARegisters.hpp"

#define HEADER_SIZE 0

class SharedRingBuffer {
 public:
//  SharedRingBuffer();
//  SharedRingBuffer(char * addr_, const int size);
  SharedRingBuffer(const int data_size, const int length, FPGARegisters* registers);
  virtual ~SharedRingBuffer();

  int get_start_address();
  char * data();
  int size();
  int data_size();
  int length();
  bool newDataAvailable();
  void movePointer();

 private:
  int data_size_;
  int length_;
  int position_;

  int allocated_memory_size_;
  int fpga_device;
  char * mmap_start_address_;
  int buffer_pysical_address_;

  FPGARegisters* registers_;
};

#endif /* SHARED_RING_BUFFER_HPP_ */
