#!/bin/sh

MENU="
1   update the boot image
2   Deploy/update rescue Linux
3   Deploy/update development Linux (NAND) preparation
4   Deploy/update development Linux (NAND) flash filesystem
5   quit
"


# Set up device and read from it.
PORT=/dev/ttyUSB0
sudo chmod 777 $PORT
( stty 115200; cat; )& < PORT

# Capture PID of background process so it is possible
# to terminate background process once writing is done
bgPid=$?

#find out the server ip address
IP_ADD=`ifconfig | awk -F':' '/inet addr/&&!/127.0.0.1/{split($2,_," ");print _[1]}'`
#echo $IP_ADD


while true; do
  clear
  echo "$MENU"
  echo -n "Please make your choice: "
  read INPUT # Read user input and assign it to variable INPUT
#### load and flash image over tftp ####

# Set up a trap in case script is killed or crashes.
trap timeout 14
timeout() {
   echo "timeout occurred"
}
pid=$$
( sleep 60 ; kill -14 $pid; )& # send alarm signal after 60 sec.

case $INPUT in
    1)
# send commands and print also to stdout
echo " " | tee $PORT
echo "setenv ipaddr '192.168.1.100'" | tee $PORT
echo "setenv serverip $IP_ADD" | tee $PORT

# update of the boot image
echo "mw.b 0x200000 0xFF 0x450000" | tee $PORT
echo "tftp 0x200000 u-boot.bin" | tee $PORT
echo "sf probe" | tee $PORT
echo "sf erase 0 0x450000" | tee $PORT
echo "sf write 0x200000 0 0x450000" | tee $PORT
    echo press ENTER to continue
    read
    ;;
    2)

# Deploy/update rescue Linux

echo "mw.b 0x00200000 0xFF ${kernel_size}" | tee $PORT
echo "tftp 0x200000 ${kernel_image}" | tee $PORT
echo "sf probe" | tee $PORT
echo "sf erase 0x00600000 ${kernel_size}" | tee $PORT
echo "sf write 0x00200000 0x600000 ${kernel_size}" | tee $PORT
echo "mw.b 0x00200000 0xFF ${devicetree_size}" | tee $PORT
echo "tftp 0x200000 ${devicetree_image}" | tee $PORT
echo "sf probe" | tee $PORT
echo "sf erase 0x900000 ${devicetree_size}" | tee $PORT
echo "sf write 0x200000 0x900000 ${devicetree_size}" | tee $PORT
echo "mw.b 0x00200000 0xFF ${ramdisk_size}" | tee $PORT
echo "tftp 0x200000 ${ramdisk_image}" | tee $PORT
echo "sf probe" | tee $PORT
echo "sf erase 0x00A00000 ${ramdisk_size}" | tee $PORT
echo "sf write 0x00200000 0x00A00000 ${ramdisk_size}" | tee $PORT
    echo press ENTER to continue
    read
    ;;

    3)
# Deploy/update development Linux (NAND)

echo "nand erase.part nand-linux" | tee $PORT
echo "ubi part nand-linux" | tee $PORT
echo "ubi create kernel" | tee $PORT
echo "nand erase.part nand-device-tree" | tee $PORT
echo "ubi part nand-device-tree" | tee $PORT
echo "ubi create dtb" | tee $PORT
echo "tftp 0x200000 ${kernel_image}" | tee $PORT
echo "ubi part nand-linux" | tee $PORT
echo "ubi write 0x200000 kernel ${filesize}" | tee $PORT
echo "tftp 0x200000 ${devicetree_image}" | tee $PORT
echo "ubi part nand-device-tree" | tee $PORT
echo "ubi write 0x200000 dtb ${filesize}" | tee $PORT
    echo press ENTER to continue
    read
    ;;

    4)
# reboot
echo " " | tee $PORT
echo "ifconfig eth0 192.168.100.100" | tee $PORT
echo "cd /tmp" | tee $PORT
echo "tftp -g -r dev-image-nm.rootfs.ubi 192.168.100.2" | tee $PORT
echo "ubiformat /dev/mtd2 -y -f /tmp/dev-image-nm.rootfs.ubi" | tee $PORT
echo "ubiattach /dev/ubi_ctrl -m 2" | tee $PORT
echo "halt" | tee $PORT
    echo press ENTER to continue
    read
    ;;
    5|q|Q) # If user presses 3, q or Q we terminate
    exit 0
    ;;
    *) # All other user input results in an usage message
    clear
    echo not a valid selection
    ;;
esac

done

# Terminate background read process
kill $bgPid
 
