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
#include "ring_buffer.hpp"


SharedMemory::SharedMemory()
: addr_(0),
  size_(0) {
}

SharedMemory::SharedMemory(char* addr_, const int size)
: addr_(addr_),
  size_(size) {
}

SharedMemory::~SharedMemory() {
}

char* SharedMemory::data() {
  return  addr_ + HEADER_SIZE;
}

int SharedMemory::size() {
  return size_;
}

bool SharedMemory::newDataAvailable() {
  return *((bool *) addr_);
}

void SharedMemory::setNewDataAvailable(bool value) {
  *((bool *) addr_) = value;
}
