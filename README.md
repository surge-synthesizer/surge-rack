# SurgeXT in Rack

This repository builds SurgeXT Rack extensions, our reboot of the old Surge 1.7 Rack modules
with more features, better design, and lots of other exciting stuff, which is a work in progress.

If you are looking for the source of the Surge modules in the VCV Rack library please check the `release/2.1.7.0`
branch.

These modules are still incomplete. We are planning an official launch but have decided to develop
them in the open. But as such we ask a few things

If you use these modules please note that, before our official release, we may at any time make breaking changes, rename
or change modules, and so on. These should not be considered stable. But they are starting to work reasonably well and
our checklist is getting shorter.

If you package modules into various forks of VCV Rack, you are welcome to do a test build, but please
do not release these modules into a distribution of a fork until we have our official 1.0 release. (Of course
the GPL3 license would allow you to ignore this request, but please don't).
[Subscribe to this issue](https://github.com/surge-synthesizer/surge-rack/issues/360)
if you want to know when the release happens.

If you want to help with the code please hit us up in the #surge-rack channel in our discord.

Still interested? Well we have a binary [Here](https://github.com/surge-synthesizer/surge-rack/releases/tag/Nightly)
which we update on every commit, so it is the latest greatest. And you can build the standard way:

```
git clone https://github.com/surge-synthesizer/surge-rack/
cd surge-rack
git submodule update --init --recursive
export RACK_DIR=location-of-rack-SDK-or-source
make dist
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
