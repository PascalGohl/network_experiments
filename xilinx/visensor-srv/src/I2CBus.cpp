#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>


#include "I2CBus.h"

I2CBus::I2CBus(const char * deviceName)
{
	fd = open(deviceName, O_RDWR);
	if (fd == -1)
	{
		throw printf("Failed to open I2C device.");
	}
}

I2CBus::~I2CBus()
{
	close(fd);
}

void I2CBus::addressSet(unsigned char address)
{
	int result = ioctl(fd, I2C_SLAVE, address);
	if (result == -1)
	{
		throw printf("Failed to set address.");
	}
}

int I2CBus::writeReg(unsigned char reg_addr, unsigned char *data){

    unsigned char buff[3];
    int retVal = -1;
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[1];

    buff[0] = reg_addr;
    buff[1] = data[0];
    buff[2] = data[1];

    messages[0].addr = deviceAddress;
    messages[0].flags = 0;
    messages[0].len = sizeof(buff);
    messages[0].buf = buff;

    packets.msgs = messages;
    packets.nmsgs = 1;

    retVal = ioctl(fd, I2C_RDWR, &packets);
    if(retVal < 0)
        perror("Write to I2C Device failed");

    return retVal;
}

int I2CBus::readReg(unsigned char reg_addr, unsigned char &data){

    unsigned char *inbuff, outbuff;
    int retVal = -1;
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[2];

    outbuff = reg_addr;
    messages[0].addr = deviceAddress;
    messages[0].flags= 0;
    messages[0].len = sizeof(outbuff);
    messages[0].buf = &outbuff;

    inbuff = &data;
    messages[1].addr = deviceAddress;
    messages[1].flags = I2C_M_RD;
    messages[1].len = sizeof(inbuff);
    messages[1].buf = inbuff;

    packets.msgs = messages;
    packets.nmsgs = 2;

    retVal = ioctl(fd, I2C_RDWR, &packets);
    if(retVal < 0)
        perror("Read from I2C Device failed");

    return retVal;
}

