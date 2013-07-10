#!/bin/sh
sudo rmmod memory_allocator
sudo insmod memory_allocator.ko
sudo chmod 777 /dev/slam-sensor
