#!/bin/bash
make
sudo insmod mycode.ko
mnum=$(cat /proc/devices | grep bank | tr -dc '0-9')

sudo mknod /dev/mycode c $mnum 0
sudo python3 pytest.py
sudo rm /dev/mycode
sudo rmmod mycode.ko
make clean
