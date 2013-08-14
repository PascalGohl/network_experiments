/*
 * FPGAConfig.cpp
 *
 *  Created on: Jul 10, 2013
 *      Author: pascal
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "FPGARegisters.hpp"

FPGARegisters::FPGARegisters(long start_address, const int size)
: dev_base_(start_address),
  size_(size) {

  // open memory device
  memfd = open("/dev/mem", O_RDWR | O_SYNC);
  if (memfd == -1) {
    printf("Can't open /dev/mem.\n");
    exit(0);
  }
  //	printf("/dev/mem opened.\n");

  // Map one page of memory into user space such that the device is in that page, but it may not
  // be at the start of the page.

  mapped_base = mmap(0, size_, PROT_READ | PROT_WRITE, MAP_SHARED, memfd,
                     dev_base_ & ~(size_ - 1));
  if (mapped_base == (void *) -1) {
    printf("Can't map the memory to user space.\n");
    exit(0);
  }

  // get the address of the device in user space which will be an offset from the base
  // that was mapped as memory is mapped at the start of a page

  mapped_dev_base = mapped_base + (dev_base_ & (size_ - 1));
}

FPGARegisters::~FPGARegisters() {
  // unmap the memory before exiting

  if (memfd != 0) {
    if (munmap(mapped_base, size_) == -1) {
      printf("Can't unmap memory from user space.\n");
      exit(0);
    }
  }
  close(memfd);
}

void FPGARegisters::print_register_values() {
  printf("==============================\n");
  printf("Print %d FPGA Registers starting at: Ox%08X\n", size_/4, dev_base_);
  printf("Offset  Hex         Int\n");
  for(int i = 0; i < size_; i+=4)
  {
    printf("0x%04X: 0x%08X  %d\n", i, getRegisterValue(i), getRegisterValue(i));
  }
  printf("==============================\n");
}

int FPGARegisters::getRegisterValue(off_t offset) {
  int val = *((volatile int *) (mapped_dev_base + offset));
  return val;
}

void FPGARegisters::setRegisterValue(off_t offset, const int value) {
  *((volatile int *) (mapped_dev_base + offset)) = value;
}
