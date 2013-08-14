/*
 * MT9V034.cpp
 *
 *  Created on: Jul 15013
 *      Author: pascal
 */

#include "MT9V034.hpp"
#include <cstdlib>
#include <stdio.h>

//#define IMAGE_SIZE 100 // debug value
#define IMAGE_SIZE 360960 //=752*480
#define BUFFER_LENGTH 8

MT9V034::MT9V034(int register_address)
: Sensor(register_address, CAM_REGISTER_BYTES, IMAGE_SIZE, BUFFER_LENGTH),
  i2c_bus() {

  registers_.setRegisterValue(BUFFER_ADDRESS, buffer_.get_start_address());
  registers_.setRegisterValue(BUFFER_SIZE, IMAGE_SIZE*BUFFER_LENGTH);
  registers_.setRegisterValue(BUFFER_LINUX_POINTER, buffer_.get_start_address() + IMAGE_SIZE*(BUFFER_LENGTH+1)); // set pointer behind the last
  registers_.print_register_values();
}

MT9V034::~MT9V034() {
  power_off();
}

void MT9V034::power_on() {
	i2c_bus.writeReg(0xB2,0x0000);    //enable lvds clock
	i2c_bus.writeReg(0xB6,0x0001);    //use 10 bit per pixel
	i2c_bus.writeReg(0xB1,0x0000);    //lvds control (not strictly necessary)
	i2c_bus.writeReg(0x20,0x03C7);    // dangerous reserved register :) more appropriate values according to "TN-09-225: MT9V024 Snapshot Exposure Mode Operation"
//  i2c_bus.writeReg(0x7F, 1 << 12 | 1 << 13); // set test pattern
//	i2c_bus.writeReg(0x7F, 0 << 12 | 0 << 13); // unset test pattern
	i2c_bus.writeReg(0x0C,0x0001);    //soft reset
////

	printf("==============================\n");
	printf("Check camera register values:\n");
	int16_t data;
	i2c_bus.readReg(0xB2, data);
	printf("Register 0xB2 read value: 0x%04X\n", data);
	i2c_bus.readReg(0xB6, data);
	printf("Register 0xB6 read value: 0x%04X\n", data);
	i2c_bus.readReg(0xB1, data);
	printf("Register 0xB1 read value: 0x%04X\n", data);
	i2c_bus.readReg(0x20, data);
	printf("Register 0x20 read value: 0x%04X\n", data);
	i2c_bus.readReg(0x7F, data);
	printf("Register 0x7F read value: 0x%04X\n", data);
//	i2c_bus.writeReg(0xB2,0x0000);    //enable lvds clock
	printf("==============================\n");

	// start FPGA reading camera
  registers_.setRegisterValue(CONTROL, 0x01);
}

void MT9V034::power_off() {
  // stop FPGA reading camera
  registers_.setRegisterValue(CONTROL, 0x00);
}
