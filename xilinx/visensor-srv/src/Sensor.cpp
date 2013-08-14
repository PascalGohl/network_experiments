/*
 * Sensor.cpp
 *
 *  Created on: Aug 14, 2013
 *      Author: pascal
 */

#include "Sensor.hpp"

//Sensor::Sensor()
//: registers(0x7e200000, 16),
//  buffer_(IMAGE_SIZE, 8){
////  registers.setRegisterValue(CAM_BUFFER_ADDRESS, buffer_.get_start_address());
////  registers.setRegisterValue(CAM_BUFFER_SIZE, IMAGE_SIZE*8);
////  registers.setRegisterValue(CAM_CONTROL, 0x01);
////  registers.print_register_values();
//}

Sensor::Sensor(long register_start_address, const int size, const int data_size,
               const int length)
: registers_(register_start_address, size),
  buffer_(data_size, length, &registers_){
}

Sensor::~Sensor() {
}

char* Sensor::data() {
  return buffer_.data();
}

int Sensor::size() {
  return buffer_.size();
}

int Sensor::data_size() {
  return buffer_.data_size();
}

void Sensor::movePointer() {
  buffer_.movePointer();
}

bool Sensor::newDataAvailable() {
  return buffer_.newDataAvailable();
}
