#!/bin/sh

echo "Executing command: make"

cd /driver
make

echo "Adding the cnn_driver module to kernel"
insmod cnn_driver.ko

cd.. 
cd /app 
gcc -o app app.c

./app weights.txt image0.txt
