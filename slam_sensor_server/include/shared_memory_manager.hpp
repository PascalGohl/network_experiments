/*
 * shared_memory.hpp
 *
 *  Created on: Jun 17, 2013
 *      Author: pascal
 */

#ifndef SHARED_MEMORY_MANAGER_HPP_
#define SHARED_MEMORY_MANAGER_HPP_

#include <vector>

#include "shared_memory.hpp"

class SharedMemoryManager {
 public:
  SharedMemoryManager(const int num_cams, const int image_size, const int imu_size,
               const int config_size);
  virtual ~SharedMemoryManager();

  int getNumCams(){
    return num_cams_;
  }

  SharedMemory& cam(const int cam_id);
  SharedMemory& imu();
  SharedMemory& config();

 private:
  int num_cams_;
  std::vector<SharedMemory> cams_;
  SharedMemory imu_;
  SharedMemory config_;
};

#endif /* SHARED_MEMORY_MANAGER_HPP_ */
