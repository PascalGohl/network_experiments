#!/bin/sh
sudo rmmod visensor
sudo insmod files/visensor.ko
sudo chmod 777 /dev/slam-sensor
