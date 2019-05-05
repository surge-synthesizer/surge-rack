# Surge in Rack (experimental)

This repo holds an experimental set of code which aims to break apart the surge synth at
https://github.com/surge-synthesizer/surge/ into distinct rack modules for the oscillator,
filter, effect, and waveshaper block segments. The best way to learn about this is to join
the Surge slack which is detailed at [the surge readme](https://github.com/surge-synthesizer/surge/blob/master/README.md)

## Developing these plugins

We are basically following [the surge developer standards](https://github.com/surge-synthesizer/surge/blob/master/doc/Developer%20Guide.md) 
just without the 3-space thing (we are using 4 spaces, no hard tabs) and using
[the surge git protocols](https://github.com/surge-synthesizer/surge/blob/master/doc/git-howto.md). We have a `.clang-format` file for 
your formatting pleasure also.

We've started writing a [basic code setup guide](docs/arch.md).

## Rack Versions

The code builds with either RACK 0.6.2 or RACK 1.0, inasmuch as it builds at all. The Makefile
automatically detects which one you have, assuming that if you are using 0.6.2 you are using the
Rack SDK and if you are using 1.0 you have checked out the v1 branch.

Before you build either system you need to have the rack environment setup. That means you need
to install MSYS2 and a few other packages. 
This is completely documented in the Rack manual:
https://vcvrack.com/manual/Building.html


If you have never built a rack plugin before, let us again suggest hopping on our slack in case you 
get stuck. We build every commit windows, mac, and linux against V0.6.2 and regularly develop against v1.

### Rack 0.6.2

Basically build against the Rack SDK as normal. The azure-pipelines.yml shows you how to do this. 
Here's an example.

In the MSYS2 64 bit shell do the following:

```
mkdir directory-of-your-choosing
cd directory-of-your-choosing
curl -o Rack-SDK.zip  https://vcvrack.com/downloads/Rack-SDK-0.6.2.zip
unzip Rack-SDK.zip
git clone https://github.com/surge-synthesizer/surge-rack
cd surge-rack
git submodule update --init --recursive
RACK_DIR=../Rack-SDK make dist
```

and you should get a built plugin. Move the plugin zip to your rack documents folder and restart rack.

### Rack 1.0

Rack 1.0 still isn't stable. But it's what @baconpaul is developing against for now so it should work.
Inasmuch as any of this alpha software works yet!

```
cd (working directory)
git clone https://github.com/VCVRack/Rack.git
cd Rack
git checkout v1
git submodule update --init --recursive
make dep
make
cd plugins
git clone https://github.com/VCVRack/Fundamental.git
cd Fundamental
git checkout v1
make dist
cd ..
git clone https://github.com/surge-synthesizer/surge-rack.git 
cd surge-rack
git submodule update --init --recursive
make dist
cd ../..
make run
```


