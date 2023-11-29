## 2.2.0 

### New Modules
  - Bonsai
    - Bonsai is a tape-style distortion effect
  - Nimbus
    - Nimbus is the surge port of the popular mutable granular delay effect
  - Digital RingModulators and Combinators
    - Surge XT 1.3 added a variety of functional forms for
      digital ring modulation and waveform combination. This module
      exposes them to the Rack ecosystem.
  - Unison Helper
    - Given a polyphonic VCO, apply the surge
      unison, drift, and character algorithms
    - Also includes an expander to route CV along the same paths
    - [This video](https://www.youtube.com/watch?v=w0fXLlHKLrk) gives you a brief guide
  - A copy of the Mixer, just with Sliders
    - Because [sometimes you feel like a knob, sometimes you don't.](https://www.youtube.com/watch?v=4b80vzwnJ8A)

### New Behaviors and Features

- Port right mouse buttons have all sorts of neat things I need to document
- EGxVCA EOC can send a trigger on any transition not just EOC, or on
  all transitions
- The Ring Modulator can modulate against external input, not just the
  internal sin oscillator
- FM2/3/Sine VCO get vintage (averaged) feedback mode. FM2/3 get optional
  extended feedback (which is always on in Sine)
- VCOs, VCF and WaveShaper can select the polyphonic channel to display on
  the internal scope / curve display

### Bug Fixes and Small Changes

- Correct The Mixer Tooltip labels
- Adjust the path to SIMDE to be absolute path
- Remove some uneeded pre-XT content from the vcvplugin build step
- Align the EGxVCA envelope/meter split with the knobs. Once you see it
  you just gotta fix it!
- Modulating a frequncy-style surge parameter gets a 'set to 1oct/v' menu item
  in the RMB.
- Cache the WT conversion for saving into your patch in the WT/Window VCO