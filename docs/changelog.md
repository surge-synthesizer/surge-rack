# SurgeRack VCV Modules Changelog


## Release 1.beta1.1 from 1.beta1.0

1.beta1.1 fixes some immediate problems ahead of the Rack 1.2 release but still leaves a collection
of issues outstanding ahead of a non-beta release.

* Add the SurgeNoise module, which implements the noise channel of the Surge VST Mixer.
* Fix input mappings in SurgeEQ, causing problems in duplication, connecting, CV Control, and leading
  to exceptions in Rack.
* Make the configuration state of parameters in the FX modules the default setting, resulting in
  right-mouse/initialize working on all the FX modules.
* Fix a problem with preset scanning in FX units by using the rack-standard preset mechanism.
* Only scan wavetables when you load the WTOSC module, not when you load other modules.

## Up to release 1.beta1.0

* 1.beta1.0 was the first release, containing the basics modules and functionality as documented
  in the manual.
