cmake_minimum_required(VERSION 2.8)
PROJECT(slam_sensor_server)
 
find_package(Boost COMPONENTS system thread REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})

include_directories (include) 

add_executable(main 
src/main.cpp 
src/tcp_server.cpp 
src/udp_server.cpp 
src/shared_memory_manager.cpp 
src/shared_memory.cpp
src/FPGA_config.cpp
)
target_link_libraries( main ${Boost_LIBRARIES} )
