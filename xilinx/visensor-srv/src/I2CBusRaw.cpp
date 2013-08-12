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

#include "I2CBusRaw.h"

// from: http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.faqs/ka10391.html
#define LOWBYTE(v)   ((unsigned char) (v))
#define HIGHBYTE(v)  ((unsigned char) (((unsigned int) (v)) >> 8))

I2CBusRaw::I2CBusRaw(void) :
mem(0x41600000, 256),
deviceAddress(0x48) {

	// reset
	mem.write_word(0x040, 0xa);

	//	1. Set RX_FIFO depth to maximum by setting RX_FIFO_PIRQ=0x0F
	mem.write_word(0x120, 0x0F);
	//	2. Set 7 bit address mode
	//	mem.write_word(0x, 0x);
	//	3. Reset TX_FIFO
	mem.write_word(0x100, 1<<1);
	//	4. Enable AXI IIC, remove TX_FIFO reset, disable general call
	mem.write_word(0x100, 0x01);
}


I2CBusRaw::~I2CBusRaw() {
}

//void I2CBusRaw::addressSet(unsigned char address)
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
int I2CBusRaw::writeReg(unsigned char reg_addr, int16_t data) {
	//	if(mem.read_word(0x104) & (1 << 6))
	//		printf("rx empty\n");
	//	if(mem.read_word(0x104) & (1 << 7))
	//		printf("tx empty\n");


	struct timespec slptm;
	slptm.tv_sec = 0;
	slptm.tv_nsec = 10000;
	for(int i = 0; !(mem.read_word(0x104) & (1 << 7)) && i < 10; i++)
	{
		//		printf("waiting for empty\n");
		nanosleep(&slptm,NULL);
	}

	for(int i = 0; (mem.read_word(0x104) & (1 << 2)) && i < 10; i++)
	{
		//		printf("waiting for not busy\n");
		nanosleep(&slptm,NULL);
	}

	if(!(mem.read_word(0x104) & (1 << 7)))
	{
		printf("I2C Bus failure.\n");
		return 1;
	}

	//slave address, start bit
	mem.write_word(0x108, deviceAddress<<1 | 1<<8);
	// write register address
	mem.write_word(0x108, reg_addr);
	// set control register (init sending)
	//	mem.write_word(0x100, 0x0D);
	// write lower data
	mem.write_word(0x108, HIGHBYTE(data));
	// write upper data
	mem.write_word(0x108,LOWBYTE(data) | 1<<9);
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
int I2CBusRaw::readReg(unsigned char reg_addr, int16_t &data) {
	// reset
	//	mem.write_word(0x040, 0xa);
	//	if(mem.read_word(0x104) & (1 << 6))
	//		printf("rx empty\n");
	//	if(mem.read_word(0x104) & (1 << 7))
	//		printf("tx empty\n");

	struct timespec slptm;
	slptm.tv_sec = 0;
	slptm.tv_nsec = 1000;
	for(int i = 0; !(mem.read_word(0x104) & (1 << 7)) && i < 10; i++)
	{
		//		printf("waiting for empty\n");
		nanosleep(&slptm,NULL);
	}

	for(int i = 0; (mem.read_word(0x104) & (1 << 2)) && i < 10; i++)
	{
		//		printf("waiting for not busy\n");
		nanosleep(&slptm,NULL);
	}


	if(!(mem.read_word(0x104) & (1 << 7)))
	{
		printf("I2C Bus failure.\n");
		return 1;
	}

	//slave address, start bit
	mem.write_word(0x108, deviceAddress<<1 | 1<<8);
	// write register address
	mem.write_word(0x108, reg_addr);
	// set control register (init sending)
	//	mem.write_word(0x100, 0x0D);

	for(int i = 0; !(mem.read_word(0x104) & (1 << 7)) && i < 10; i++)
	{
		//		printf("1waiting for empty\n");
		nanosleep(&slptm,NULL);
	}
	//slave address read bit, start bit
	mem.write_word(0x108, deviceAddress<<1 | 0x01 | 1<<8);

	for(int i = 0; !(mem.read_word(0x104) & (1 << 7)) && i < 10; i++)
	{
		//		printf("2waiting for empty\n");
		nanosleep(&slptm,NULL);
	}
	// config for reading 2 bytes and than stop
	mem.write_word(0x108, 0x02 | 1<<9);

	//		printf("rx empty: %x\n", (mem.read_word(0x104) & (1 << 6)));
	for(int i = 0; (mem.read_word(0x104) & (1 << 6)) && i < 10; i++)
	{
		//			printf("3waiting for empty\n");
		nanosleep(&slptm,NULL);
	}
	int hi = mem.read_word(0x10C);

	//		printf("rx empty: %x\n", (mem.read_word(0x104) & (1 << 6)));
	for(int i = 0; (mem.read_word(0x104) & (1 << 6)) && i < 10; i++)
	{
		//			printf("3waiting for empty\n");
		nanosleep(&slptm,NULL);
	}
	int lo = mem.read_word(0x10C);

	data = lo | (hi<<8);

	return 0;
}
//
//int I2CBusRaw::openI2C() {
//	i2cDescriptor = open(i2cFileName.c_str(), O_RDWR);
//
//	if (i2cDescriptor == -1) {
//		throw printf("Failed to open I2C device.");
//		exit(-1);
//	}
//
//	if (ioctl(i2cDescriptor, I2C_SLAVE, deviceAddress) < 0) {
//		printf("Cannot set I2C address\n");
//		exit(1);
//	}
//
//	return i2cDescriptor;
//}
//
//int I2CBusRaw::closeI2C() {
//	int retVal = -1;
//	retVal = close(i2cDescriptor);
//	if(retVal < 0){
//		perror("Could not close file (1)");
//		exit(-1);
//	}
//	return retVal;
//}
