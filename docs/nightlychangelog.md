## 2.2.2

A small fix release after 2.2.1

- LFO Uni- and Bi-polar waveforms align properly.
- Randomization on the CXOR no longer goes out of bounds on type
- Quad LFO Independent Mode triggers corrected so they use individual
  triggers as opposed to mistakenly using trigger 0
- Allow code to build in some non-VCV-rack compiler and configuration settings
- Upgrade the connector library to allow in-row connection
- Some tweaks to the ringmod, including changing display when connecting and
  disconnecting
- FX sidechains participate in neighbor inputs
- RingMod deactivates useless controls when sideband attached


