#!/bin/sh

RACK_DIR=/Users/paul/dev/VCVRack/V1SDK/Rack-SDK
STRIP=echo

if [ ! -f build/rack1.info ]; then
	echo "Can't find rack6.info; cleaning for you"
	RACK_DIR=${RACK_DIR} make clean
	mkdir build
	echo date > build/rack1.info
fi

RACK_DIR=${RACK_DIR} make -j 4 -k dist || exit 2
