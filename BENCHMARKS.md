# Real-hardware benchmarks

All figures below were measured on a real Nokia 9110, not extrapolated from the PC-hosted SDK emulator.

## Measurement rules

- Let each mode stabilize for at least 8–10 seconds where possible.
- Record guest FPS and blit/display FPS separately.
- Compare modes inside the same binary whenever possible.
- Keep the same gameplay scene and avoid input during A/B samples.
- Verify visual correctness and input after every renderer or CPU change.
- Profiler FPS includes counter overhead and is not a clean speed benchmark.
- Static blit tests contain no emulation and measure only the GEOS transfer path.

## Milestone table

| Build / experiment | CPU/core | Renderer / display | Complete or practical path | Result |
|---|---:|---:|---:|---|
| GBPROF / GBREN baseline | 21–22 | packed 6 | full 5 | Initial decomposition |
| GBFAST v0.5 | 21 | packed 6 | full 5 | Direct packing did not help |
| GBTABLE v0.6 | 22 | packed 12 | full 9 | First decisive renderer win |
| GBHOT v0.8 | 25 | packed 13 | full 10 | Fast ROM/stack/CB paths |
| GBTIME v0.9 | 26–27 | packed 13 | full 10 | Small housekeeping gain |
| GBFLAGS v1.0 | 27 | packed 14 | full 10 | Smaller code, limited speed gain |
| GBDIV v1.1 | 28 with DIV | — | full 10 | First guarded superinstruction |
| GBTILE v1.2 | — | packed 11 | full 8 | Cache regression |
| GBROW v1.3 | — | no-blit 17 | full 12 | Eight-pixel renderer |
| GBROW v1.3 + DIV | 28 | — | **full 13** | Best render-every-frame path |
| GBFS v1.4c FS2 | — | display 9 | **guest 18** | Best practical fixed-skip balance |
| GBFS v1.4c FS3 | — | display 7 | guest 21 | Faster guest, visible slideshow |
| GBDIV2 v1.6 | **28** | FS2 display 9 | FS2 guest **18** | Lower division fallback rate |

Static GEOS 160×144 4-bpp blit remains about **44 FPS**.

## Stage decomposition

Early baseline:

```text
CPU only                            21–22 FPS
Original PPU only                       7 FPS
Original PPU + 4-bpp packing            6 FPS
Complete original path                  5 FPS
Static GEOS 4-bpp blit                 44 FPS
```

Current clean paths:

```text
Optimized CPU + DIV v2                  28 FPS
ROW8 renderer, no blit                  17 FPS
ROW8 render-every-frame path            12 FPS
DIV v1 + ROW8 full path                 13 FPS
FS2 + DIV v2                    18 guest / 9 display FPS
Static GEOS 4-bpp blit                 44 FPS
```

## Frame-skip experiment: GBFS v1.4c

Guest execution continues every frame. Only rendering and blitting are skipped together.

| Mode | Guest FPS | Blit/display FPS | Guest gain vs FULL |
|---|---:|---:|---:|
| FULL | 13 | 13 | — |
| FS2 | 18 | 9 | +38% |
| FS3 | 21 | 7 | +62% |

FS2 is the useful compromise. FS3 is mainly diagnostic because 7 displayed FPS is visibly coarse.

## Live profiler: GBPROF v1.5j

### Opcode-core window

```text
Guest FPS: 26
OP/s:      28,126
DIV/s:        870
```

| Opcode | Meaning | Per second | Share |
|---:|---|---:|---:|
| `20` | `JR NZ,r8` | 3,775 | 13% |
| `F0` | `LDH A,(a8)` | 2,228 | 7% |
| `80` | `ADD A,B` | 1,720 | 6% |
| `E6` | `AND d8` | 1,247 | 4% |
| `C6` | `ADD A,d8` | 1,195 | 4% |
| `28` | `JR Z,r8` | 990 | 3% |
| `E0` | `LDH (a8),A` | 954 | 3% |
| `21` | `LD HL,d16` | 840 | 3% |

The hot set changes when rendering and profiling are enabled together, so this is guidance for a narrow A/B assembly experiment, not proof that eight isolated handlers should be rewritten blindly.

### Data-memory window

Opcode/immediate fetch bytes are excluded.

```text
Guest FPS: 24
Reads/s:   19,135
Writes/s:   7,759
DIV/s:        848
```

| Region | Reads/s | Writes/s |
|---|---:|---:|
| WRAM | 12,408 | 6,862 |
| HRAM | 4,657 | 69 |
| I/O | 2,070 | 828 |

### Combined profiler + FS2

```text
Guest FPS: 15
Blit FPS:   7
OP/s:      22,501
Reads/s:   11,114
Writes/s:   6,145
DIV/s:        224
```

These FPS numbers include both opcode and memory counters and must not be compared with clean FS2.

## DIV v2 A/B benchmark

DIV v2 retains the original long `FULL` helper and adds shorter `PREFIX` and `FINAL` blocks.

### CPU-only

| Mode | Guest FPS | FULL/s | PREFIX/s | FINAL/s | FALLBACK/s |
|---|---:|---:|---:|---:|---:|
| OFF | 26 | 0 | 0 | 0 | 0 |
| DIV v1 | 27 | 403 | 0 | 0 | 929 |
| DIV v2 | **28** | 413 | 635 | 54 | **350** |

### FS2

| Mode | Guest FPS | Blit FPS | FULL/s | PREFIX/s | FINAL/s | FALLBACK/s |
|---|---:|---:|---:|---:|---:|---:|
| OFF | 17 | 6 | 0 | 0 | 0 | 0 |
| DIV v1 | 16 | 9 | 268 | 0 | 0 | 619 |
| DIV v2 | **18** | **9** | 272 | 415 | 35 | **229** |

The single FS2 v1 sample is slower in guest FPS than the OFF sample, so it should not be over-interpreted. The robust result is that DIV v2 preserved the 18/9 practical rate while reducing fallback entries by about 63% relative to DIV v1.

All captured samples reported `CORE ERR = 0` and `DIV SIG = OK`.

## What the numbers mean

### Direct packing was not enough

Removing a framebuffer copy left the renderer at 6 FPS. The inner algorithm, not just the copy, was expensive.

### Four-pixel LUT and ROW8 worked

`GBTABLE v0.6` doubled packed throughput. ROW8 then halved the background-loop iteration count and moved no-blit rendering from 13 to 17 FPS.

### CPU changes accumulated gradually

Fast instruction fetch, direct WRAM stack access, hot CB handlers, packed flags, timing cleanup and the guarded division helpers raised core throughput from roughly 22 to 28 FPS.

### Frame skip is an architectural gain

The full path spends material time in CPU, renderer and GEOS transfer. Skipping renderer and blit together on selected guest frames therefore raises guest progress more than another small isolated renderer tweak.

### A perfect cache hit rate can still be slower

`GBTILE v1.2` produced 99.98–100% cache hits yet reduced packed performance from 13 to 11 FPS and full performance from 10 to 8 FPS.

### DIV v2 closed the remaining large C superinstruction opportunity

Shorter guarded blocks reduced normal-interpreter fallback entries by roughly 62–63%. The remaining CPU work is broad enough that the next experiment should be a small assembly fast path with a C reference mode, not another large ROM-specific helper.

## Overall improvement

```text
First measured full path:         5 FPS
Best render-every-frame path:    13 FPS
Best current practical FS2:      18 guest / 9 display FPS
CPU-only path:                   28 FPS
```

## Reporting template

```text
Device:
Build:
Mode:
Guest FPS:
Display/blit FPS:
Helper or profiler counters:
Visual correctness:
Input correctness:
Run duration:
Warnings or anomalies:
```
