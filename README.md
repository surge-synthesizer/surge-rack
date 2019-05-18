# Surge in Rack (experimental)

This repo holds an experimental set of code which aims to break apart the surge synth at
https://github.com/surge-synthesizer/surge/ into distinct rack modules for the oscillator,
filter, effect, and waveshaper block segments. The best way to learn about this is to join
the Surge slack which is detailed at [the surge readme](https://github.com/surge-synthesizer/surge/blob/master/README.md)

**This is Alpha software, changing frequently, including modules with incomplete or non-functional
panels and DSP code and streaming fragility. There is no guarantee that the code here will work, 
will compile, or will save and load a patch from session to session or version to version.**

That being said, we do run a v0.6.2 build in continuous integration against every PR, and our primary development
is mac v1, so there is a good chance you can get it to build. If you see an error please raise a github issue.

## What are the plugins provided and their state

Our overall goal for our first release is to have all the stages exposed as modules which work with an acceptable
front plate, but (1) are still monophonic and (2) don't support temposync for parameters, just absolute values.
Here's how far we are along on that journey.

<table>
<tr><th>Plugin</th><th>Description</th><th>State</th></tr>
<tr><td>SurgeFX (mulitple modules)</td>
  <td>The surge FX stage is represented as a set of modules, one per effect.</td>
  <td>The modules all work; most have a generic panel still</td></tr>
<tr><td>SurgeADSR</td><td>The Surge Envelope Generator with digital and analog modes</td><td>Completed</td></tr>
<tr><td>SurgeOSC</td><td>The Surge non-wavetable Oscillators</td><td>Very good shape. Few pops and clicks when changing osc type or changing unison count, but works well otherwise.</td></tr>
<tr><td>SurgeWaveShaper</td><td>The waveshaper stage, including digital, tanh, warmers</td><td>Completed</td></tr>
<tr><td>SurgeWTOSC</td><td>The WaveTable Oscillator</td><td>Innumerable problems. Try at your own risk</td></tr>
<tr><td>SurgeLFO</td><td>The powerful surge LFO</td><td>Works perfectly; terrible UI</td></tr>
<tr><td>SurgeVCF</td><td>The amazingly amazing Surge Filters</td><td>which don't yet exist in rack</td></tr>
<tr><td>SurgeVOC</td><td>The surge Vocoder stage</td><td>which doesn't yet exist in rack</td></tr>
</table>

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

### Before you build

Before you build either system you need to have the rack environment setup. 
On windows, that means you need
to install MSYS2 and a few other packages. On Mac, xcode-select and a few things.
On Linux a few apt-gets.

This is completely documented in the Rack manual:
https://vcvrack.com/manual/Building.html

If you have never built a rack plugin before, let us again suggest hopping on our slack in case you 
get stuck. We build every commit windows, mac, and linux against V0.6.2 and regularly develop against v1.

### Rack 0.6.2

Basically build against the Rack SDK as normal. The azure-pipelines.yml shows you how to do this. 
Here's an example.

On Windows: start a MSYS2 64. On Linux and Mac start a terminal of your choosing.

**Very first time you build**

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

**Subsequent changes**

```
cd directory-of-your-choosing/surge-rack
(change code as desired)
RACK_DIR=../Rack-SDK make dist
```

and you should get a built plugin. Move the plugin zip to your rack documents folder and restart rack.

### Rack 1.0

Rack 1.0 still isn't stable. But it's what @baconpaul is developing against for now so it should work.
Inasmuch as any of this alpha software works yet!

**First Time**

```
cd (working directory)
git clone https://github.com/VCVRack/Rack.git
cd Rack
git checkout v1-gpl
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

**Subsequent Changes**

```
cd (working directory)/Rack/plugins/surge-rack
(change code)
make dist && ( cd ../.. ; make run )
```

## Updating your working copy

The [the surge git protocols](https://github.com/surge-synthesizer/surge/blob/master/doc/git-howto.md) have full instructions
but just as a refresher 

**If you cloned from surge-synthesizer/surge-rack**

```
git checkout master
git pull origin master
```

**If you have your own fork**

*one time*

```
git remote add upstream https://github.com/surge-synthesizer/surge-rack.git
```

*When you want to update*

```
git checkout master
git fetch usptream
git reset upstream/master --hard
git push origin
```
