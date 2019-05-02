#!/bin/sh

RACK_DIR=/Users/paul/dev/VCVRack/V1/Rack/
STRIP=echo

if [ ! -f build/rack1.info ]; then
	echo "Can't find rack6.info; cleaning for you"
	RACK_DIR=${RACK_DIR} make clean
	mkdir build
	echo date > build/rack1.info
fi

RACK_DIR=${RACK_DIR} make -j 4 -k dist
cp dist/SurgeRack-1.0.0-mac.zip ${RACK_DIR}/plugins
pushd ${RACK_DIR}/plugins
rm -rf SurgeRack
unzip SurgeRack-1.0.0-mac.zip
cd ${RACK_DIR}
ls ./plugins/SurgeRack/surge-data
make run
popd

