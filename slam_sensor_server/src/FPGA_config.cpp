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

#include "FPGA_config.h"

/*
FPGA Register layout:

0 Bytes         4 Bytes           8 Bytes           12 Bytes            16 Bytes
 ----------------------------------------------------------------------------
|   Data Buffer Mem. Addr.         |   Buffer Size     |  Number of Cameras  |
 ----------------------------------------------------------------------------
|   Camera Rate  |    IMU Rate     |
 ----------------------------------------------------------------------------
 */

#define BASE_ADDRESS     0x41200000
#define DATA_OFFSET     0
#define DIRECTION_OFFSET     4

#define MAP_SIZE 16
#define MAP_MASK (MAP_SIZE - 1)

FPGAConfig::FPGAConfig()
    : dev_base(BASE_ADDRESS) {
  memfd = open("/dev/mem", O_RDWR | O_SYNC);
  if (memfd == -1) {
    printf("Can't open /dev/mem.\n");
    exit(0);
  }
  printf("/dev/mem opened.\n");

  // Map one page of memory into user space such that the device is in that page, but it may not
  // be at the start of the page.

  mapped_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, memfd,
                     dev_base & ~MAP_MASK);
  if (mapped_base == (void *) -1) {
    printf("Can't map the memory to user space.\n");
    exit(0);
  }
  printf("Memory mapped at address %p.\n", mapped_base);

  // get the address of the device in user space which will be an offset from the base
  // that was mapped as memory is mapped at the start of a page

  mapped_dev_base = mapped_base + (dev_base & MAP_MASK);
}

FPGAConfig::~FPGAConfig() {
  // unmap the memory before exiting

  if (munmap(mapped_base, MAP_SIZE) == -1) {
    printf("Can't unmap memory from user space.\n");
    exit(0);
  }

  close(memfd);
}

void FPGAConfig::print_config() {
  printf("Current FPGA config:\n");
  printf("Data Buffer Address: %lu\n", get_data_buffer_address());
  printf("Data Buffer Size: %d\n", get_data_buffer_size());
  printf("Number of Cameras: %d\n", get_num_cams());
}

unsigned long int FPGAConfig::get_data_buffer_address() {
  unsigned long val = *((volatile unsigned long *) (mapped_dev_base));
  return val;
}

void FPGAConfig::set_data_buffer_address(const unsigned long int add) {
  *((volatile unsigned long *) (mapped_dev_base)) = add;
}

int FPGAConfig::get_data_buffer_size() {
  int val = *((volatile int *) (mapped_dev_base + 8));
  return val;
}

void FPGAConfig::set_data_buffer_size(const int size) {
}

int FPGAConfig::get_num_cams() {
  int val = *((volatile int *) (mapped_dev_base + 12));
  return val;
}

void FPGAConfig::set_num_cams(const int num_cams) {
}
