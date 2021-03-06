/*
 * shared_memory.hpp
 *
 *  Created on: Jun 17, 2013
 *      Author: pascal
 */

#ifndef SHARED_MEMORY_HPP_
#define SHARED_MEMORY_HPP_

#define HEADER_SIZE 1

class SharedMemory {
 public:
  SharedMemory();
  SharedMemory(char * addr_, const int size);
  virtual ~SharedMemory();

  char * data();
  int size();
  bool newDataAvailable();
  void setNewDataAvailable(bool value);

 private:
  int size_;
  char * addr_;
};

#endif /* SHARED_MEMORY_HPP_ */
