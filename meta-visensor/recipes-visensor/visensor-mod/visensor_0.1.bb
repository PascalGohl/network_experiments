DESCRIPTION = "Driver for the aslam-sensor FPGA interface."
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=12f884d2ae1ff87c09e5b7ccc2c4ca7e"

inherit module

PR = "r0"
PV = "0.2"

SRC_URI = "file://Makefile \
           file://visensor.c \
           file://COPYING \
          "

S = "${WORKDIR}"
