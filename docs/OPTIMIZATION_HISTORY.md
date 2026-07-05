# Optimization history

## Baseline: 5 FPS complete path

The first real-hardware profiler measured:

```text
CPU only                 21–22 FPS
Original PPU                  7 FPS
PPU + packing                 6 FPS
Complete 4-bpp path           5 FPS
Static 4-bpp blit            44 FPS
```

The requested timer rate was not the problem. The work itself exceeded the frame period.

## GBFAST v0.5 — useful failure

The frontend wrote directly into a packed framebuffer and removed a copy. Hardware performance remained:

```text
packed  6 FPS
full    5 FPS
```

Conclusion: the inner renderer algorithm dominated.

## GBTABLE v0.6 — first decisive win

Changes:

- four background pixels per LUT group;
- packed writes;
- sprite lists prepared per scanline;
- fewer irrelevant sprite checks.

Result:

```text
packed  6 → 12 FPS
full    5 →  9 FPS
```

## GBCPU v0.7 — measure the interpreter

A real-device profiler found:

- half of main instructions in ten opcodes;
- 95% of CB instructions in four rotate commands;
- 75% of reads in fixed ROM;
- most writes in WRAM.

## GBHOT v0.8 — CPU hot paths

Changes:

- direct ROM fetch;
- direct WRAM stack;
- hot CB operations;
- high-memory read path.

Result:

```text
core        22 → 25 FPS
table pack  12 → 13 FPS
full         9 → 10 FPS
```

## GBTIME v0.9 — housekeeping cleanup

Changes included interrupt/timer simplification, fewer call boundaries, and disabling unused RTC/serial work for the test ROM.

Result:

```text
core 25 → 26–27 FPS
full stayed 10 FPS
```

Useful, but not a new major bottleneck.

## GBFLAGS v1.0 — packed flag register

The LR35902 `F` register became one byte with masks rather than per-flag Borland bit fields.

Result:

```text
core        ~27 FPS
table pack   14 FPS
full         10 FPS
```

Code size fell materially, but speed changed only slightly.

## GBDIV v1.1 — guarded superinstruction

An exact ROM signature identifies a hot division-loop iteration. The native path preserves guest registers, stack effects, cycles, and falls back when unsafe.

A/B result inside one binary:

```text
base core  26 FPS
DIV core   28 FPS
```

Around 25 fast iterations execute per guest frame.

## GBTILE v1.2 — cache regression

A decoded tile-row cache reported:

```text
99.98–100% cache hits
```

Yet performance fell:

```text
table pack  13 → 11 FPS
full        10 →  8 FPS
```

The lookup and reconstruction cost exceeded the original LUT decoder. High hit rate alone was not evidence of a useful cache.

## GBROW v1.3 — halve the background loop

ROW8 processes:

```text
20 groups × 8 pixels
```

instead of:

```text
40 groups × 4 pixels
```

`SCX & 7` is selected once per line, previous tile data is reused, and no large cache is involved.

Result:

```text
packed           13 → 17 FPS
full             10 → 12 FPS
DIV + ROW8       10 → 13 FPS
```

## Current conclusion

The full path improved from 5 to 13 FPS, or 2.6×. The next practical gain should come from decoupling guest execution and display frequency rather than rendering every guest frame.
