# Surge XT VCV Modules Changelog

## 2.1 - In Pre-Beta Now (this as of c314e1e1f037 / Dec 2)

- New Modules
    - EGxVCA (still in development)
    - QuadAD (still very much in development)

- Module Changes
    - The *Waveshaper* has a DC Blocker. It is on by default in new instances but 
      waveshapers saved with 2.0 releases will laod with the blocker off to
      retain compatability.

- Module UI Changes
    - The *LFO* step editor is substantially better, allowing cross bar
      drags and other gestures
    - The *Alias* harmonic editor is similarly improved
    - *LFO* randomization of shape is constrained so a step sequencer
      never randomizes off of a step setting.

- Bug Fixes
    - The modulation arm button didn't consume events, leading it to work 
       inconsistently in module lock mode

- Infrastructure and Code Cleanups
    - Skip the airwindows sub-library build in the surge dep phase since we dont use it in rack
    - Centralize some more implementations of clock UI and processing elements
    - Upgrade all our github actions to SDK 2.2 including an ARM cross compile
    - Add a blank module in docs and a little script to bring it to life with a name

## 2.0.3 - Nov 26, 2022

- Module Features and Behaviors
    - **Mixer**: Module unmutes channel 2 or 3 the first time you connect an input to its ports
    - **TunedDelay**:
        - polyphony is set by either the inputs or by v/oct
        - monophonic audio signals are broadcast to multiple delay lines if given a polyphonic v/oct
    - **TreeMonster**: Added a v/oct and env output from the internal state so the pitch tracking and envelope  
      following can be used in other contexts
    - **String VCO**: Fix a memory leak that a delay line per trigger would leak since they were not
      re-used or returned to the pre-allocated pool on retrigger.

- UI
    - Substantially improve the modulation tooltips for Surge parameters
    - VCF panel preview now says "FILTER" not "FILTERS"
    - Modulations on the LFOxEG module update the wave in response to modulated values
    - Correct the gradient on LFOxEG dark mode panel.

- Other Changes
    - use `std::fopen`/`json_loadf` rather than `json_loadfile` to load skin defaults
    - Add a widget and Skin for self-drawing output region backgrounds
    - Activate ARM builds and make build with ARM
    - Make work with Rack SDK 2.2.0

## 2.0.2 - Unreleased

## 2.0.1 - Initial Release
