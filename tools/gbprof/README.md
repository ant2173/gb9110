# GBPROF v0.3

Stage profiler for real Nokia 9110 hardware.

It isolates:

- CPU/core;
- PPU rendering;
- 4-bpp packing;
- complete frame output;
- static 4-bpp and 1-bpp GEOS blits.

This profiler established the first hardware baseline and showed that the early 3 FPS behavior was workload-bound rather than a timer-rate bug.
