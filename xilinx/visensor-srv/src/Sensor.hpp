/*
 * Sensor.hpp
 *
 *  Created on: Aug 14, 2013
 *      Author: pascal
 */

#ifndef SENSOR_HPP_
#define SENSOR_HPP_

#include "FPGARegisters.hpp"
#include "SharedRingBuffer.hpp"

class Sensor {
 public:
  Sensor(long register_start_address, const int size, const int data_size, const int length);
  virtual ~Sensor();

  char * data();
  int size();
  int data_size();
  void movePointer();
  bool newDataAvailable();

 protected:
  FPGARegisters registers_;
  SharedRingBuffer buffer_;
};

#endif /* SENSOR_HPP_ */
