# Real-hardware benchmarks

All figures below were measured on a real Nokia 9110, not extrapolated from the PC-hosted SDK emulator.

## Measurement rules

- Let each mode stabilize for at least 5–10 seconds.
- Record guest FPS and blit/display FPS separately where available.
- Compare modes inside the same binary whenever possible.
- Verify visual correctness after every renderer change.
- Static blit tests contain no emulation and measure only the GEOS transfer path.

## Milestone table

| Build / experiment | CPU/core | Packed renderer | Full path | Static 4-bpp blit | Result |
|---|---:|---:|---:|---:|---|
| GBPROF / GBREN baseline | 21–22 | 6 | 5 | 44 | Initial hardware decomposition |
| GBFAST v0.5 | 21 | 6 | 5 | 44 | Direct packing did not help |
| GBTABLE v0.6 | 22 | 12 | 9 | 44 | First decisive renderer win |
| GBHOT v0.8 | 25 | 13 | 10 | 44 | Fast ROM/stack/CB paths |
| GBTIME v0.9 | 26–27 | 13 | 10 | 44 | Small housekeeping gain |
| GBFLAGS v1.0 | 27 | 14 | 10 | 44 | Smaller code, limited speed gain |
| GBDIV v1.1 | 28 with DIV | — | 10 | 44 | Superinstruction helps CPU |
| GBTILE v1.2 | — | 11 | 8 | 44 | Tile cache regression |
| GBROW v1.3 | — | 17 | 12 | 44 | Eight-pixel renderer |
| GBROW v1.3 + DIV | 28 | — | **13** | 44 | Current best complete path |

## Stage decomposition

Early baseline:

```text
CPU only                            21–22 FPS
Original PPU only                       7 FPS
Original PPU + 4-bpp packing            6 FPS
Complete original path                  5 FPS
Static GEOS 4-bpp blit                 44 FPS
```

Current best:

```text
Optimized CPU + DIV                     28 FPS
ROW8 renderer, no blit                  17 FPS
ROW8 full path                          12 FPS
DIV + ROW8 full path                    13 FPS
Static GEOS 4-bpp blit                 44 FPS
```

## What the numbers mean

### Direct packing was not enough

Removing a framebuffer copy left the renderer at 6 FPS. The inner algorithm, not just the copy, was expensive.

### Four-pixel LUT rendering worked

`GBTABLE v0.6` processed four background pixels per group and prepared sprite lists per line. Packed throughput doubled from 6 to 12 FPS; the full path rose from 5 to 9 FPS.

### CPU changes accumulated gradually

Fast instruction fetch, direct WRAM stack access, hot CB handlers, packed flags, and timing cleanup raised core throughput from roughly 22 to 27 FPS.

### The division superinstruction is real but bounded

Inside one binary, baseline core measured 26 FPS and DIV core 28 FPS. The fast path triggered around 25 times per guest frame, but safety checks intentionally leave some iterations to the normal interpreter.

### A perfect cache hit rate can still be slower

`GBTILE v1.2` produced 99.98–100% cache hits yet reduced packed performance from 13 to 11 FPS and full performance from 10 to 8 FPS. Pointer arithmetic, dirty checks, counters, and cache-row reconstruction cost more than the compact LUT decoder.

### Reducing loop count worked

ROW8 changed background work from 40 four-pixel groups to 20 eight-pixel groups per scanline. Packed performance rose from 13 to 17 FPS and the complete path from 10 to 12 FPS. Combining ROW8 with DIV reached 13 FPS.

## Overall improvement

```text
First measured full path:   5 FPS
Current best full path:    13 FPS
Speedup:                  2.6×
```

This is not full Game Boy speed, but it is a substantial measured improvement on unchanged hardware.

## Reporting template

```text
Device:
Build:
Mode:
Guest FPS:
Display/blit FPS:
Visual correctness:
Input correctness:
Run duration:
Warnings or anomalies:
```
