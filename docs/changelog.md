# Surge XT VCV Modules Changelog

## 2.1 - In Beta Now (As of Jan 11, 2023 / 4d2133d)

- New Modules
    - *EGxVCA* - a combined ADSR or DAHD envelope with a panning stereo VCA
    - *QuadAD* - four AD / AR envelopes with various settings
    - *QuadLFO* - four LFOs of various shapes with various inter-relationships
    - *Tuned Delay +* - an expanded version of the Tuned Delay
       with feedback, filters, modulation targets, and more.

- Module Changes
    - The *Waveshaper* has a DC Blocker. It is on by default in new instances but 
      waveshapers saved with 2.0 releases will laod with the blocker off to
      retain compatability.
    - The *Twist Oscillator* LPG implementation fixed a variety of problems, including
      incorrect delay scaling and over-resetting the oscillator.
    - The *Exciter* effect correctly scales its inputs and outputs, avoiding overflows
      and NaNs but potentially lowering the excitation level from 2.0.3.
    - The *Delay* module allows users to select a variety of internal clipping models and 
      the internal clipping model default is changed to a more appropriate for rack setting
      of +/- 10v hardclip.
    - The plugin no longer ships with all the wavetables but instead only contains a basic
      subset. To restore the third party wavetables use "download extra content" in the WT or
      window wavetable menu.

- Module UI Changes
    - The *LFO* step editor is substantially better, allowing cross bar
      drags and other gestures
    - The *Alias* harmonic editor is similarly improved
    - *LFO* randomization of shape is constrained so a step sequencer
      never randomizes off of a step setting.

- Bug Fixes
    - The delay module Clock in pulse-clock mode had an incorrect sample rate leading to
      incorrect temposyncing. (Temposync in BPM mode was correct).
    - The modulation arm button didn't consume events, leading it to work 
       inconsistently in module lock mode
    - Twist Oscillator didn't reset sample rate correctly on single process SR change
    - The modulation display in VCO tooltips would change when modulated
      which is obviously incorrect! Change to base modulation depths off of underlying
      value to correct, like all the other modules.
    - Modulation depth typeins for some surge quantities were scaled incorrectly.
    - The Alias oscillator no longer allows selection of the memory locations, which
      are not useful in the rack context
    - The ModMatrix matrix display paints negative modulations correctly.

- Infrastructure and Code Cleanups
    - Skip the airwindows sub-library build in the surge dep phase since we dont use it in rack
    - Centralize some more implementations of clock UI and processing elements
    - Upgrade all our github actions to SDK 2.2.2 including an ARM cross compile
    - Add a blank module in docs and a little script to bring it to life with a name
    - In a mis-installed SurgeXT with no wavetables, make wavetable VCO act as sine oscillator
    - Compile correctly if a build forces AVX flags
    - The output region markers on panels are now painted in software not the SVGs.
    - FX do not scan wavetables
    - Move the build to CMake; Run with the 2.2.2 SDK

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
