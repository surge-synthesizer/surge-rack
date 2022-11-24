# Surge XT VCV Modules Changelog

## 2.1 - In Beta Now (this as of a97451f2 / Nov 23)

- New Modules
    - EGxVCA (still in development)

- Module Features and Behaviors
    - **Mixer**: Module unmutes channel 2 or 3 the first time you connect an input to its ports
    - **TunedDelay**:
        - polyphony is set by either the inputs or by v/oct
        - monophonic audio signals are broadcast to multiple delay lines if given a polyphonic v/oct
    - **TreeMonster**: Added a v/oct and env output from the internal state so the pitch tracking and envelope  
      following can be used in other contexts

- UI
    - Substantially improve the modulation tooltips for Surge parameters
    - Correct an error with the modulation tooltip that, in some cases, would show the
      modulation value as base value
    - VCF panel preview now says "FILTER" not "FILTERS"
    - Modulations on the LFOxEG module update the wave in response to modulated values

- Other Changes
    - use `std::fopen`/`json_loadf` rather than `json_loadfile` to load skin defaults
    - Add a widget and Skin for self-drawing output region backgrounds

## 2.0.1 - Nov 17, 2022

2.0.1 was the initial release of Surge XT for Rack.
