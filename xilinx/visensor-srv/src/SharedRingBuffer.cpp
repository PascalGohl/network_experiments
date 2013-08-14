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
#include <sys/mman.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <boost/foreach.hpp>

#include "SharedRingBuffer.hpp"

SharedRingBuffer::SharedRingBuffer(const int data_size, const int length, FPGARegisters* registers)
    : data_size_(data_size),
      length_(length),
      position_(0),
      registers_(registers){

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

  read(fpga_device, &buffer_pysical_address_, 0);
}

SharedRingBuffer::~SharedRingBuffer() {
}

char* SharedRingBuffer::data() {
  return mmap_start_address_ + position_*data_size_;
}

int SharedRingBuffer::size() {
  return data_size_ * length_;
}

bool SharedRingBuffer::newDataAvailable() {
  // reag fpga pointer
  int fpga_pointer = registers_->getRegisterValue(BUFFER_FPGA_POINTER);

  // get position of next data
  int next_data_position = position_+1;
  if(next_data_position >= length_+1)
    next_data_position = 0;

  // compare if fpga is still there or has new data
  if(buffer_pysical_address_ + data_size_*next_data_position == fpga_pointer)
  {
//    printf("fpga pointer is on next data: %p == %p\n", fpga_pointer, buffer_pysical_address_ + data_size_*next_data_position);
    return false;
  }
//  printf("fpga pointer is not on next data: %d == %d\n", (fpga_pointer - buffer_pysical_address_)/data_size_, next_data_position);
  return true;
}

int SharedRingBuffer::length() {
  return length_;
}

int SharedRingBuffer::data_size() {
  return data_size_;
}

int SharedRingBuffer::get_start_address() {
  return buffer_pysical_address_;
}

void SharedRingBuffer::movePointer() {
  position_++;
  if(position_ >= length_)
      position_ = 0;

  // move the pointer in the register for blocking the FPGA
  registers_->setRegisterValue(BUFFER_LINUX_POINTER, buffer_pysical_address_ + data_size_*position_);
}
