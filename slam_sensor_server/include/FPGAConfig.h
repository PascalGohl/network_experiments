/*
 * FPGAConfig.h
 *
 *  Created on: Jul 10, 2013
 *      Author: pascal
 */

#ifndef FPGACONFIG_H_
#define FPGACONFIG_H_

class FPGAConfig {
 public:
  FPGAConfig();
  virtual ~FPGAConfig();

  void print_config();

 private:
  int memfd;
  void *mapped_base, *mapped_dev_base;
  off_t dev_base;
};

#endif /* FPGACONFIG_H_ */
