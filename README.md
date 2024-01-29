# SurgeXT for VCV Rack

This repository builds SurgeXT Rack modules.

It has tags for each official release (including tags for the prior Surge 1.7
releases) as well as the current beta on the main branch.

By far the easiest way to use these modules is

1. Install VCV Rack and log into the library
2. Go to the "Surge XT" plugin page in the library [here](https://library.vcvrack.com/SurgeXTRack)
3. Click the "subscribe" button in the upper corner. We recommend all users subscribe 
to the entire plugin and not module by module, since the modules work as a suite
4. Restart VCV rack and update your library.

In addition to versions in the library, we provide user downloadable binaries
[here](https://github.com/surge-synthesizer/surge-rack/tags), including the Nightly
which is our current beta.

If you are looking for the source of the prior Surge modules in the VCV Rack library please check the `release/2.1.7.0`
branch.

## How to develop with these modules

Checkout sources:
```
git clone https://github.com/surge-synthesizer/surge-rack
cd surge-rack
git submodule update --init --recursive
```

Option 1: Use standard rack make
```
export RACK_DIR=location-of-rack-SDK-or-source
make dep
make dist
```

Option 2: Build with CMake:
```
mkdir surge-rack-build
cd surge-rack-build
cmake -DCMAKE_INSTALL_PREFIX=dist -DRACK_SDK_DIR=location-of-rack-SDK-or-source ../surge-rack
make install
```

To compile the plugin with CMake for Mac OSX `x86_64` platform append `-DCMAKE_OSX_ARCHITECTURES="x86_64"` to the cmake
command, to compile for `arm64` platform use `-DCMAKE_OSX_ARCHITECTURES="arm64"`.


Alternatively create a plugin release with Make and point to an existing cmake build location:
```
CMAKE_BUILD=location-of-cmake-build-dir make dist
```

## License and Copyright

This software is licensed under the Gnu General Public License v3 or later.
The license is included in the file "LICENSE" in this repository.

It is a derived work of both Surge
(which is GPL3 or later) and VCVRack (which is GPL3 or later) and
their dependencies.

It contains the quicksand font which is licensed under OpenFontLicense 1.1.

The SVGs used (in res/xt) are distributed under the Creative Commons CC-BY-NC-SA-4.0 and are
copyright Steve and Pierre.

Copyright to this software is held by the authors with authorship indicated by the github transaction log.
