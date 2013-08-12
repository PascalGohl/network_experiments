#ifndef _I2CBusRaw_h
#define _I2CBusRaw_h

#include <stdint.h>
#include <string>

#include "DevMem.h"

class I2CBusRaw
{
public:
	I2CBusRaw(void);
//	I2CBusRaw(unsigned char dev_addr, std::string i2cfilename);

	~I2CBusRaw();

	int writeReg(unsigned char reg_addr, int16_t data);
	int readReg(unsigned char reg_addr, int16_t &data);

private:
//	int openI2C(); //open an I2C device. Called only in constructor
//	int closeI2C(); // close an I2C device. Called only in destructor

	DevMem mem;
	unsigned char deviceAddress; // i2c device address
};

#endif
