#!/bin/sh

echo "Executing command: make"

cd /root/cnn_driver/driver
make

echo "Adding the cnn_driver module to kernel"
insmod cnn_driver.ko

cd .. 
cd /root/cnn_driver/app 

echo "Building the app"
gcc -o app app.c

echo "Running the app"
./app weights.txt image0.txt
