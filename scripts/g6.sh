#!/bin/sh

RACK_DIR=/Users/paul/dev/VCVRack/build/Rack-SDK/
STRIP=echo

if [ ! -f build/rack6.info ]; then
	echo "Can't find rack6.info; cleaning for you"
	RACK_DIR=${RACK_DIR} make clean
	mkdir build
	echo date > build/rack6.info
fi

RACK_DIR=${RACK_DIR} make -j 4 -k go
