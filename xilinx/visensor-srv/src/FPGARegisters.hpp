/*
 * FPGAConfig.h
 *
 *  Created on: Jul 10, 2013
 *      Author: pascal
 */

#ifndef FPGACONFIG_H_
#define FPGACONFIG_H_

#include <stdio.h>

// FPGA Registers:
#define FPGA_SOFTWARE_VERSION  0x0000  // a increment of 100 means new incompatible interface
// Hardware register start addresses
#define CAM0_REGISTER_OFFSET   0X0010
#define CAM1_REGISTER_OFFSET   0X0030
#define CAM2_REGISTER_OFFSET   0X0050
#define CAM3_REGISTER_OFFSET   0X0070

// ring buffer registers
#define CONTROL                0x0000   // | 0: ENABLE | 1:....
#define BUFFER_ADDRESS         0x0004
#define BUFFER_SIZE            0x0008
#define BUFFER_FPGA_POINTER    0x000c
#define BUFFER_LINUX_POINTER   0x0010

// camera specific registers
#define CAM_REGISTER_BYTES     32
#define CAM_TRIGGER_FREQUENCY  0x0014
#define CAM_RESERVED1          0x0018
#define CAM_RESERVED2          0x001C

class FPGARegisters {
 public:
  FPGARegisters(long start_address, const int size /*in Bytes*/);
  virtual ~FPGARegisters();

  int getRegisterValue(off_t offset /*in Bytes*/);
  void setRegisterValue(off_t offset /*in Bytes*/, const int value);

  void print_register_values();

 private:
  int memfd;
  void *mapped_base, *mapped_dev_base;
  off_t dev_base_;
  int size_;
};

#endif /* FPGACONFIG_H_ */
