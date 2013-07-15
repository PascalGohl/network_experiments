#ifndef _I2CBus_h
#define _I2CBus_h

#include <stdint.h>

class I2CBus
{
public:
    I2CBus(const char * deviceName);
    ~I2CBus();

    void addressSet(unsigned char address);
    int writeReg(unsigned char reg_addr, unsigned char *data);
    int readReg(unsigned char reg_addr, unsigned char &data);
    uint8_t readByte();

private:
    int fd;
    unsigned char deviceAddress; // i2c device address
};

#endif
