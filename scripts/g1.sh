#!/bin/sh

RACK_DIR=/Users/paul/dev/VCVRack/V1/Rack/
STRIP=echo

if [ ! -f build/rack1.info ]; then
	echo "Can't find rack6.info; cleaning for you"
	RACK_DIR=${RACK_DIR} make clean
	mkdir build
	echo date > build/rack1.info
fi

RACK_DIR=${RACK_DIR} make -j 4 -k all || exit 2
cp plugin.dylib ${RACK_DIR}/plugins/SurgeRack
# cd ${RACK_DIR}
# lldb -- ./Rack -d 
# ./Rack -d 
# exit 0
RACK_DIR=${RACK_DIR} make -j 4 -k dist || exit 2
cp dist/SurgeRack-1.0.0-mac.zip ${RACK_DIR}/plugins
cd ${RACK_DIR}/plugins
rm -rf SurgeRack
unzip SurgeRack-1.0.0-mac.zip
ls ./plugins/SurgeRack/surge-data
cd ${RACK_DIR}
./Rack -d 
