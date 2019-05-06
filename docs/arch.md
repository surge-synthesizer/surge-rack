# Surge Rack Code and Architecture

This document is under construction, but here's some basics

* I'm using a .hpp/.cpp split for all the modules
  * The .hpp defines the model class. It contains all the parameters and dsp
  * The .cpp defines the widget class. It maps to the widget
  * Semantically think "hpp makes the sound; cpp makes the UI"
* Modules interact with surge in a few ways
  * Where a surge 'object' exists (OSC, FX, ADSR, LFO) we create a SurgeStorage and use the getPatch() from
    the storage to directly address the surge Parameters. The parameters provide control names and formatting.
  * Where a surge function exists (Waveshaper, Filters) we do the equivalent work of SurgeVoice to map inputs
    to outputs
  * In all cases we have to compensate for the BLOCK_SIZE or __m128 wide calculations in the SSE2 engine, sometimes
    introducing a delay between input and output of BLOCK_SIZE or 4.
* All our modules subclass `SurgeModuleCommon` which subclasses `rack::Module` allowing us to put various
  surge-specific base class functions in place.
* We handle 0.6.2 and v1 by defining RACK_V1 or RACK_V062 at compile time and just fighting it out with ifdefs.
* Surge is compiled from the makefile with an explicit reference to the classes in the submodule.
  We compile with TARGET_RACK and TARGET_HEADLESS both true, so the surge modules look like the 
  surge headless code more or less.
* The UI is all drawn with code, not SVGs. The code uses the layout and style functions in SurgeStyle and SurgeRackGUI
  
