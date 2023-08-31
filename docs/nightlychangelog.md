## 2.2.0 (as of  80743550a31cf)

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
  - A copy of the Mixer, just with Sliders
    - Because [sometimes you feel like a knob, sometimes you don't.](https://www.youtube.com/watch?v=4b80vzwnJ8A)

### New Behaviors and Features

- Port right mouse buttons have all sorts of neat things I need to document
- EGxVCA EOC can send a trigger on any transition not just EOC, or on
  all transitions

### Bug Fixes 

- Correct The Mixer Tooltip labels
- Adjust the path to SIMDE to be absolute path