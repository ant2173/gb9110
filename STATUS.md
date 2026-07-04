# Project status

**Stage:** hardware proof of concept and profiler-driven optimization.

## Confirmed working

- GEOS app creation and deployment
- real device launch
- ROM file loading into two locked 16 KiB blocks
- Peanut-GB initialization
- LR35902 execution
- LCD scanline callback
- 160×144 GEOS framebuffer
- keyboard mapping
- clean shutdown path
- EC and NC builds
- self-queued maximum-throughput profiling

## Confirmed limitations

- current playable frontend is not full-speed on hardware
- no sound
- no persistent cartridge RAM
- ROM path and size assumptions are still hard-coded
- modified core/header is tailored to Borland C89-era constraints
- general compatibility has not been evaluated

## Latest real-hardware measurements

| Revision/test | Core | PPU | PPU + pack | Full | 4-bpp blit | 1-bpp blit |
|---|---:|---:|---:|---:|---:|---:|
| GBPROF v0.3 | 22 | — | 6 | 5 | 44 | 29 |
| GBREN v0.4 | 22 | 7 | 6 | 5 | 44 | — |
| GBFAST v0.5 | 21 | — | 6 | 5 | 44 | — |

`GBTABLE v0.6` is the current unmeasured experiment.

## Honest headline

A real Game Boy ROM now executes, renders, and accepts input on a real Nokia 9110. The current full path is about 5 FPS, and the CPU-only ceiling is about 21–22 FPS. The project is working, but performance optimization is still the central engineering problem.
