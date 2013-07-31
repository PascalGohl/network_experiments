#ifndef _I2CBus_h
#define _I2CBus_h

#include <stdint.h>
#include <string>

class I2CBus
{
public:
	I2CBus(void);
	I2CBus(unsigned char dev_addr, std::string i2cfilename);

	~I2CBus();

	int writeReg(unsigned char reg_addr, int16_t data);
	int readReg(unsigned char reg_addr, int16_t &data);

private:
	int openI2C(); //open an I2C device. Called only in constructors
	int closeI2C(); // close an I2C device. Called only in destructor

	std::string i2cFileName; //i2c device name e.g."/dev/i2c-0" or "/dev/i2c-1"
	int i2cDescriptor;
	unsigned char deviceAddress; // i2c device address
};

#endif
