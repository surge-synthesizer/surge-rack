# rm -f surge/ignore/rack-build/src/common/libsurge-common.a
RACK_DIR=../Rack-SDK make -j dep
# pushd surge/ignore/rack-build && CFLAGS= && make -j 1 surge-common
# popd

RACK_DIR=../Rack-SDK make -j dist && cp dist/SurgeRack-2.xt-1.0.1-mac.vcvplugin ~/Documents/Rack2/plugins && /Applications/VCV\ Rack\ 2\ Free.app/Contents/MacOS/Rack

