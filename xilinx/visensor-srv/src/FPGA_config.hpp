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

  int get_data_buffer_address();
  void set_data_buffer_address(const int add);
  int get_data_buffer_size();
  void set_data_buffer_size(const int size);
  int get_num_cams();
  void set_num_cams(const int num_cams);

 private:
  int memfd;
  void *mapped_base, *mapped_dev_base;
  off_t dev_base;
};

#endif /* FPGACONFIG_H_ */
