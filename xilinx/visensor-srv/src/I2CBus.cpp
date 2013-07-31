#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <iostream>
#include <cstdlib>
#include <string.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>

#include "I2CBus.h"

I2CBus::I2CBus(void) :
i2cFileName("/dev/i2c-0"), i2cDescriptor(-1), deviceAddress(0x48) {
	openI2C();
}

I2CBus::I2CBus(unsigned char dev_addr, std::string i2cfilename) :
				i2cFileName(i2cfilename), i2cDescriptor(-1), deviceAddress(dev_addr) {
	openI2C();
}

I2CBus::~I2CBus() {
}

//void I2CBus::addressSet(unsigned char address)
//{
//	int result = ioctl(fd, I2C_SLAVE, address);
//	if (result == -1)
//	{
//		throw printf("Failed to set address.");
//	}
//	deviceAddress = address;
//}

/********************************************************************
 *This function writes two bytes of data "data" to a specific register
 *"reg_addr" in the I2C device This involves sending these two bytes
 *in order to the i2C device by means of the ioctl() command. Since
 *both bytes are written (no read/write switch), both pieces
 *of information can be sent in a single message (i2c_msg structure)
 ********************************************************************/
int I2CBus::writeReg(unsigned char reg_addr, int16_t data) {
    unsigned char i2c_buffer[3];
    i2c_buffer[0] = reg_addr;
    i2c_buffer[1] = data;
    i2c_buffer[2] = data;

	if(write(i2cDescriptor, i2c_buffer, 2) != 2) {
		printf("I2C write failed\n");
		exit(1);
	}

    unsigned bytesRead;

	bytesRead = read(i2cDescriptor, i2c_buffer, 2);
	    if(bytesRead != 2) {
	        printf("I2C read failed. Return code = %u\n", bytesRead);
	        exit(1);
	    }


	//	unsigned char buff[1 + sizeof(data)];
	//	int retVal = -1;
	//	struct i2c_rdwr_ioctl_data packets;
	//	struct i2c_msg messages[1];
	//
	//	buff[0] = reg_addr;
	//	memcpy(&(buff[1]), &data, sizeof(data));
	//
	//	messages[0].addr = deviceAddress;
	//	messages[0].flags = 0;
	//	messages[0].len = sizeof(buff);
	//	messages[0].buf = buff;
	//
	//	packets.msgs = messages;
	//	packets.nmsgs = 1;
	//
	//	retVal = ioctl(i2cDescriptor, I2C_RDWR, &packets);
	//	if (retVal < 0)
	//		perror("Write to I2C Device failed");

	return 0;
}

/********************************************************************
 *This function reads two bytes of data "data" from a specific register
 *"reg_addr" in the I2C device. This involves sending the register address
 *byte "reg_Addr" with "write" asserted and then instructing the
 *I2C device to read a byte of data from that address ("read asserted").
 *This necessitates the use of two i2c_msg structs. One for the register
 *address write and another for the read from the I2C device i.e.
 *I2C_M_RD flag is set. The read data is then saved into the reference
 *variable "data".
 ********************************************************************/
int I2CBus::readReg(unsigned char reg_addr, int16_t &data) {

	unsigned char *inbuff, outbuff;
	int retVal = -1;
	struct i2c_rdwr_ioctl_data packets;
	struct i2c_msg messages[2];

	outbuff = reg_addr;
	messages[0].addr = deviceAddress;
	messages[0].flags = 0;
	messages[0].len = sizeof(outbuff);
	messages[0].buf = &outbuff;

	inbuff = (unsigned char *) &data;
	messages[1].addr = deviceAddress;
	messages[1].flags = I2C_M_RD;
	messages[1].len = sizeof(data);
	messages[1].buf = inbuff;

	packets.msgs = messages;
	packets.nmsgs = 2;

	retVal = ioctl(i2cDescriptor, I2C_RDWR, &packets);
	if (retVal < 0)
		perror("Read from I2C Device failed");

	return 0;
}

int I2CBus::openI2C() {
	i2cDescriptor = open(i2cFileName.c_str(), O_RDWR);

	if (i2cDescriptor == -1) {
		throw printf("Failed to open I2C device.");
		exit(-1);
	}

	if (ioctl(i2cDescriptor, I2C_SLAVE, deviceAddress) < 0) {
		printf("Cannot set I2C address\n");
		exit(1);
	}

	return i2cDescriptor;
}

int I2CBus::closeI2C() {
	int retVal = -1;
	retVal = close(i2cDescriptor);
	if(retVal < 0){
		perror("Could not close file (1)");
		exit(-1);
	}
	return retVal;
}
