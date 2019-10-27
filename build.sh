#!/bin/bash

./dockcross-linux-arm64 bash -c 'make' && cp kernel8.img /Volumes/boot/
sleep 1
./dockcross-linux-arm64 bash -c 'make' && cp kernel8.img /Volumes/boot/

#docker run --rm -v $(pwd):/app -w /app smatyukevich/raspberry-pi-os-builder make $1

#cp kernel8.img /Volumes/boot/
