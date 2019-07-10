# Surge in Rack

This repository houses the implementation of the [Surge Synthesizer](https://surge-synthesizer.github.io/) as
collection of modules for [VCVRack](https://vcvrack.com/). The modules are fully documented in the 
[Surge for Rack Manual](https://surge-synthesizer.github.io/rack_manual/).

The easiest way to use these modules is to download VCVRack and include the modules in the Rack Plugin Manager.
The library link is [here](https://vcvrack.com/plugins.html#Surge%20for%20Rack).

If you would like to use the latest version of the software, which is always equal to or ahead of the version in
the plugin manager, but which may not be as stable as the plugin manager version, you can get a zip file from
[our release page](https://github.com/surge-synthesizer/surge-rack/releases/tag/Nightly) and place the platform
appropriate version of the zip in your Rack `plugins-v1` directory.

If you would like to request a feature or report a bug, please 
[open a github issue](https://github.com/surge-synthesizer/surge-rack/issues)
with a clear description of the feature or bug, the operating system you use, and the version of the plugins.

Finally, if you want to build these modules, they build using the 
[standard Rack plugin building mechanism](https://vcvrack.com/manual/Building.html). Once your environment
is configured, those steps are

```
git clone https://github.com/surge-synthesizer/surge-rack/
cd surge-rack
git submodule update --init --recursive
export RACK_DIR=location-of-rack-SDK-or-source
make dist
```


## License and Copyright

This software is licensed under the Gnu Public License v3. It is a derived work of both Surge (which is GPL3)
and VCVRack 1.0 (which is VCV3). It contains the Encode font which is licensed under OpenFontLicense 1.1.
Copyright to this software is held by the authors with authorship indicated by the github transaction log.

Copyright 2019, Various authors.
