# Surge XT VCV Modules Changelog


## 2.2.1

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

- Port right mouse buttons have all sorts of neat things to auto connect
  to neighbors and to mixmaster/auxmaster
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
- In rare cases the SpringReverb could NaN. Reset the effect automatically if
  these occur as a workaround.
- The EGxVCA didn't trigger envelopes for newly added polyphony while
  the gate was high. This caused polyphony mis-fires on patch startup
  in some rare cases. Now it will trigger an envlope for a
  newly added channel in a high gate situation.
- Rework the LFOxEG gate behavior so the code is less confusing.
  As a result, fix a bug where the envelope would mis-trigger when
  both GATE and GATEENV were connected.
- VCO Character, Level and Drift don't participate in randomization

## 2.1.8

Upgrades for the Rack 2.4.0 SDK and the final
shape of the ARM64 Rack Build. (2.1.7 includes
some of these changes and was never released to
the library)

## 2.1.6

A few minor changes based on user requests

- Twist Oscillator has an option to exclude Engine from randomization
- The stereo pan options in EGxVCA are named more clearly
- Sliders show modulation animation like Knobs do
- The waveshaper has a (global) option to display either the current
  response-to-sine or the response-to-sine and also the waveshaper transform

## 2.1.5

A few minor fixes causing crashes and mis-streams

- The Modern SINE 'other' shape didn't properly unstream when saved
- In some situations stormelder strip could cause the QuadLFO to crash when 
  randomizing; change the semi-incorrect surge behaviro which trips up
  the semi-incorrect stormelder randomizer
- Don't append 'Input' and 'Output' to input and output port names, leading
  to nicer tooltips

## 2.1.4 

- EGxVCA: Pan Law is now an option of either the MixMaster
Stereo Equal Power or True Panning. Stereo Equal Power is
the new default, matching MixMaster. *This will change the
behavior of EGxVCA pan in saved patches unless you explicitly
restore the True Panning mode in the menu*.
- Mixer, TunedDelays: Left/Mono normalize properly.
- Mixer: Turn of the 'unmute-on-connect' mixer behavior when unstreaming, allowing
mute to save correctly
- Twist: Don't paint the Twist waveform with the LPG on
- LFOxEG: Imporove wording on "Set EG to Zero" on LFO
- LFOxEG: Paint the raw wave as a 'ghost' wave in the LFO display
- LFOxEG: Adjust the Phase/Shuffle label on the LFO front pane
- WaveTable/Window: Fix several problems with loading incomplete wavetables and
painting the resulting wavetable display.
- Infrastructure: Make Surge's use of SIMDE compile time selectable; use rack SIMDE

## 2.1.3

- Fix for wavetable 3d position display when modulating Morph parameter

## 2.1.2 - February 2023 (Skipped in library)

- Add a 'slow' mode to the EGxVCA which allows ADSR times up to 120 seconds
- Fix an EGxVCA problem where monophonic gates with polyphonic signals would
  mot broadcast correctly
- Refactor some of the code for sharing with other surge team properties; fix
  a few small technical make and build issues for disparate environments.

## 2.1.1 - February 2023

2.1.1 provides a few small bugfixes. 

- Supress some memory leaks in 2.1 due to mis-using `json_set_object` vs
  `json_set_object_new`. This leak motivated the point release even
  though the memory leaked was small.
- EGxVCA in DAHD Mode changes retrigger semantics slightly. If you have
  delay set at minimum the delay phase is skipped, so there is no zero
  segment at the outset; in this case a retrigger of the DAHD will
  follow the same legato controls as the ADSR. (This basically means
  "The DAHD works as you would exect in AHD mode with D at min")
- EGxVCA UI tweaks for control positions, gate time drawing, and labels
- If you have both 2.0.3 and extra wavetables installed, only scan the
  'extra' downloads copy

## 2.1 - Januar 2023

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
