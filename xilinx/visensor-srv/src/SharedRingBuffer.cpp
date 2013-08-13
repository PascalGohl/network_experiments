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
#include "SharedRingBuffer.hpp"

SharedRingBuffer::SharedRingBuffer(const int data_size, const int length)
    : addr_(0),
      data_size_(data_size),
      length_(length),
      position_(0){
}

SharedRingBuffer::~SharedRingBuffer() {
}

char* SharedRingBuffer::data() {
  return addr_ + position_*data_size_;
}

int SharedRingBuffer::size() {
  return data_size_ * length_;
}

bool SharedRingBuffer::newDataAvailable() {
  return *((bool *) addr_);
}

int SharedRingBuffer::length() {
  return length_;
}

void SharedRingBuffer::setNewDataAvailable(bool value) {
  *((bool *) addr_) = value;
}

int SharedRingBuffer::data_size() {
  return data_size_;
}

void SharedRingBuffer::movePointer() {
  position_++;
  if(position_ >= length_)
      position_ = 0;
}
