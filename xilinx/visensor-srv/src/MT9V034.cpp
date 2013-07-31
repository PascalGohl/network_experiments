/*
 * MT9V034.cpp
 *
 *  Created on: Jul 15013
 *      Author: pascal
 */

#include "MT9V034.hpp"
#include <cstdlib>
#include <stdio.h>

MT9V034::MT9V034()
: i2c_bus(0x48, "/dev/i2c-1") {
}

MT9V034::~MT9V034() {
}

void MT9V034::power_on() {
	i2c_bus.writeReg(0xB2,0x0000);    //enable lvds clock
//	i2c_bus.writeReg(0xB6,0x0001);    //use 10 bit per pixel
//	i2c_bus.writeReg(0xB1,0x0000);    //lvds control (not strictly necessary)
//	i2c_bus.writeReg(0x0C,0x0001);    //soft reset
//	i2c_bus.writeReg(0x20,0x03C7);    // dangerous reserved register :) more appropriate values according to "TN-09-225: MT9V024 Snapshot Exposure Mode Operation"
}

void MT9V034::power_off() {
}
