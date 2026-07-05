# Optimization history

## Baseline — 5 FPS complete path

```text
CPU only                 21–22 FPS
Original PPU                  7 FPS
PPU + packing                 6 FPS
Complete 4-bpp path           5 FPS
Static 4-bpp blit            44 FPS
```

The requested timer rate was not the problem. The work itself exceeded the frame period.

## GBFAST v0.5 — useful failure

Direct packed output removed a copy but left performance at 6 FPS packed and 5 FPS full. The inner renderer algorithm dominated.

## GBTABLE v0.6 — first decisive win

Four background pixels per LUT group and prepared sprite lists raised packed throughput from 6 to 12 FPS and the full path from 5 to 9 FPS.

## GBCPU v0.7 — measure the interpreter

The first real-device profiler found a concentrated opcode set, fixed-ROM fetch traffic, WRAM stack traffic and a hot rotate-based division loop.

## GBHOT v0.8 — CPU hot paths

Direct ROM fetch, direct WRAM stack, hot CB operations and high-memory fast paths raised core throughput from 22 to 25 FPS and full throughput from 9 to 10 FPS.

## GBTIME v0.9 — housekeeping cleanup

Interrupt/timer simplification and fewer call boundaries moved core to 26–27 FPS but left the full path at 10 FPS.

## GBFLAGS v1.0 — packed flag register

The LR35902 `F` register became one byte with masks rather than per-flag Borland bit fields. Code size fell materially; speed changed only slightly.

## GBDIV v1.1 — first guarded superinstruction

An exact ROM signature identifies a hot division-loop iteration. A/B core throughput measured 26 FPS baseline and 28 FPS with the helper.

## GBTILE v1.2 — cache regression

A decoded tile-row cache reported 99.98–100% hits yet reduced packed performance from 13 to 11 FPS and full performance from 10 to 8 FPS. The lookup path cost more than compact decoding.

## GBROW v1.3 — halve the background loop

ROW8 processes 20 groups × 8 pixels instead of 40 groups × 4 pixels.

```text
packed           13 → 17 FPS
full             10 → 12 FPS
DIV + ROW8       10 → 13 FPS
```

## GBFS v1.4c — guest time and display time diverge

Rendering and blitting are skipped together while guest execution continues every frame.

```text
FULL  13 guest / 13 display FPS
FS2   18 guest /  9 display FPS
FS3   21 guest /  7 display FPS
```

FS2 became the practical fixed mode. FS3 demonstrated the remaining graphics cost but looked too coarse.

## GBPROF v1.5j — profile the current workload

The new rate-based profiler measured the post-ROW8 workload rather than relying only on the earlier 180-frame sample.

Representative opcode-core result:

```text
28,126 opcodes/s
JR NZ,r8             13%
LDH A,(a8)            7%
ADD A,B               6%
```

Representative data-memory result, excluding opcode fetch:

```text
19,135 reads/s
 7,759 writes/s
WRAM dominates both directions
```

The profiler also exposed an engineering issue in the old SDK: a huge Borland local-symbol stream could crash Glue. The final build disables debug scopes only around the included engine, preserving GOC class symbols.

## GBDIV2 v1.6 — shorter safe blocks

DIV v2 keeps the original complete helper and adds 48-cycle `PREFIX` and 52-cycle `FINAL` helpers.

CPU-only A/B:

```text
OFF       26 FPS
DIV v1    27 FPS, 929 fallback/s
DIV v2    28 FPS, 350 fallback/s
```

FS2 A/B sample:

```text
OFF       17 guest / 6 display FPS
DIV v1    16 guest / 9 display FPS, 619 fallback/s
DIV v2    18 guest / 9 display FPS, 229 fallback/s
```

The stable conclusion is not that every individual FPS sample is perfectly monotonic; it is that the shorter blocks reduced fallback by roughly 62–63%, retained correctness checks and preserved the best practical 18/9 rate.

## Current conclusion

The project improved the render-every-frame path from 5 to 13 FPS and the practical FS2 path to 18 guest / 9 display FPS. The broad C opportunities are now diminishing. The next experiment is a narrow, reversible 8086 assembly fast path with same-binary C comparison.
