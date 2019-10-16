#!/bin/bash

docker run --rm -v $(pwd):/app -w /app smatyukevich/raspberry-pi-os-builder make $1

cp kernel8.img /Volumes/boot/
