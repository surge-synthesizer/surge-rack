# Surge in Rack (experimental)

This repo holds an experimental set of code which aims to break apart the surge synth at
https://github.com/surge-synthesizer/surge/ into distinct rack modules for the oscillator,
filter, effect, and waveshaper block segments. The best way to learn about this is to join
the Surge slack which is detailed at [the surge readme](https://github.com/surge-synthesizer/surge/blob/master/README.md)

**This is Alpha software, changing frequently, including modules with incomplete or non-functional
panels and DSP code and streaming fragility. There is no guarantee that the code here will work, 
will compile, or will save and load a patch from session to session or version to version.**

As of May 19, the weekend of the dev release of Rack 1.0, we have stopped supporting 0.6.2 and now have a
1.0 only codebase. Mode details on this are below.

## What are the plugins provided and their state

Our overall goal for our first release is to have all the stages exposed as modules which work with an acceptable
front plate, are full "phase2/3" rack 1.0 modules, but are still monophonic.
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
<tr><td>SurgeVOC</td><td>The surge Vocoder stage</td><td>Completed</td></tr>
</table>

## Developing these plugins

We are basically following [the surge developer standards](https://github.com/surge-synthesizer/surge/blob/master/doc/Developer%20Guide.md) 
just without the 3-space thing (we are using 4 spaces, no hard tabs) and using
[the surge git protocols](https://github.com/surge-synthesizer/surge/blob/master/doc/git-howto.md). We have a `.clang-format` file for 
your formatting pleasure also.

We've started writing a [basic code setup guide](docs/arch.md).


### Before you build

Before you build either system you need to have the rack environment setup. 
On windows, that means you need
to install MSYS2 and a few other packages. On Mac, xcode-select and a few things.
On Linux a few apt-gets.

This is completely documented in the Rack manual:
https://vcvrack.com/manual/Building.html

If you have never built a rack plugin before, let us again suggest hopping on our slack in case you 
get stuck. We build every commit windows, mac, and linux against V0.6.2 and regularly develop against v1.

### Download Rack 1.0 and the Rack 1.0 SDK

[This post from Andrew](https://community.vcvrack.com/t/rack-v1-development-blog/1149/501?u=baconpaul) explains
how to download Rack.


### Build the Plugins if you are already set up to develop Rack

Presuming you have RACK_DIR set it's standard, just you need a git submodule

**First Time**

```
cd (working directory)
git clone https://github.com/surge-synthesizer/surge-rack.git 
cd surge-rack
git submodule update --init --recursive
make dist
```

This will result in a .zip file which you can place in your working directory

**Subsequent Changes**

```
cd (working directory)/Rack/plugins/surge-rack
(change code)
make dist
```

### Using the scripts/buildutil.sh

Like `build-osx.sh` in surge, I wrote a little script to make a clean area, do builds, etc... and actually
download and run rack 1.0 configured in a way you want. The script is `scripts/buildutil.sh`. Here's a sample session. 
You can set a variable `RACK_INSTALL_DIR` for the working copy if you want. Read the script to understand it really.

**First Time Only**

```
cd (working directory)
git clone https://github.com/surge-synthesizer/surge-rack.git 
cd surge-rack
git submodule update --init --recursive
./scripts/buildutil.sh --get-rack
```

You know have rack and surge-rack ready to go

**Build and run**

```
./scripts/buildutil.sh --build
./scripts/buildutil.sh --install
./scripts/buildutil.sh --run
```

there is also the shortcut `scripts/buildinstall.sh --bir` to do all 3; or `scripts/buildinstall.sh --br` to build and 
run and only install the dll (not assets).



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


## What about rack 0.6.2

For a while, when we weren't sure whether surge-rack would finish before rack 1.0, we were building these
plugins for both versions with a spaghetti hell of ifdefs. We stopped that on May 19, but we checkpointed
the code at the `checkpoint/last_working_062` branch if you want to check it out.

Here at surge-rack we are super supportive of 1.0, think it is great, and appreciate the hard work. We plan to
be 1.0 only. If for some reason you would like to backport to 0.6.2, please get in touch and we can talk about
pull requests onto that branch.

## License and Copyright

This software is licensed under the Gnu Public License v3. It is a derived work of both Surge (which is GPL3)
and VCVRack 1.0 (which is VCV3). It contains the Encode font which is licensed under OpenFontLicense 1.1.
Copyright to this software is held by the authors with authorship indicated by the github transaction log.

Copyright 2019, Various authors.
