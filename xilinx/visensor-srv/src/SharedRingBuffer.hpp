/*
 * shared_memory.hpp
 *
 *  Created on: Jun 17, 2013
 *      Author: pascal
 */

#ifndef SHARED_RING_BUFFER_HPP_
#define SHARED_RING_BUFFER_HPP_

#define HEADER_SIZE 0

class SharedRingBuffer {
 public:
//  SharedRingBuffer();
//  SharedRingBuffer(char * addr_, const int size);
  SharedRingBuffer(const int data_size, const int length);
  virtual ~SharedRingBuffer();

  char * data();
  int size();
  int data_size();
  int length();
  bool newDataAvailable();
  void setNewDataAvailable(bool value);
  void movePointer();

  void setAddr(char* addr) {
    addr_ = addr;
  }

 private:
  char * addr_;
  int data_size_;
  int length_;
  int position_;
};

#endif /* SHARED_RING_BUFFER_HPP_ */
