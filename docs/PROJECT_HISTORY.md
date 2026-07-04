# Development history

## 1. GEOS foundation

The first milestones intentionally avoided emulation:

- static drawing;
- timer-driven animation;
- full 160×144 4-bpp framebuffer;
- synthetic CPU workload.

This established that GEOS drawing and scheduling worked before introducing Peanut-GB.

## 2. Peanut-GB adaptation

The upstream header targets C99-style environments. The port required work for:

- Borland C++ 4.52;
- C89-era syntax and library assumptions;
- 16-bit segmented memory;
- GEOS-specific file and heap APIs.

Raw callback pointers caused a runtime failure. Compile-time direct hooks became the stable integration method.

## 3. First real ROM frame

The ROM was first embedded, which pushed too much data into the near-data model. The successful design loads two 16 KiB banks from an external file into GEOS memory blocks.

A fresh GEOS application identity was also essential. Reusing an old permanent name/token could restore stale state and imitate a startup hang.

## 4. Continuous execution and input

A conservative 10 FPS build worked. The same design reached 30 FPS and then about 59 guest FPS in the SDK emulator. Keyboard input was added without changing the proven scheduling loop.

## 5. First real-hardware run

The program launched on an actual Nokia 9110, but:

- the whole screen visibly flickered;
- all requested speed modes converged near 3 FPS.

The flicker came from clearing and redrawing the whole status window. The identical speed across timer modes showed that work duration, not requested cadence, was the limit.

## 6. Profiler-driven optimization

A self-queued profiler separated:

- CPU;
- PPU;
- framebuffer packing;
- GEOS blit.

Results showed a 21–22 FPS CPU-only ceiling, a drop to 7 FPS with the PPU, and about 5 FPS for the complete path.

## 7. Negative result

A direct packed renderer eliminated the temporary scanline and final copy but stayed at 6 FPS. The algorithm still performed too much work and scanned all sprites per line.

## 8. Current experiment

The current lookup-table renderer attempts to:

- decode four background pixels per iteration;
- use one 16-bit store for four packed pixels;
- preselect the first ten sprites touching each scanline;
- visit only relevant sprites during line rendering.

It has not yet been measured on hardware.
