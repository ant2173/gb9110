# Changelog

## 2026-07-05 — frame skip, live profiling and DIV v2

### Added

- `experiments/gbfs`: fixed FULL / FS2 / FS3 benchmark
- `tools/gbprof-live`: rate-based opcode and data-memory profiler
- `experiments/gbdiv2`: layered division-helper A/B benchmark
- real-device profiler and DIV v2 photographs
- hardware result documents for GBFS v1.4c, GBPROF v1.5j and GBDIV2 v1.6
- first controlled 8086 assembly experiment plan

### Measured

- FULL: 13 guest / 13 display FPS
- FS2: 18 guest / 9 display FPS
- FS3: 21 guest / 7 display FPS
- current opcode-core rate: 28,126 opcodes/s
- current data-memory rate: 19,135 reads/s and 7,759 writes/s
- DIV v2 CPU-only path: 28 guest FPS
- DIV v2 fallback reduction: about 62–63% relative to DIV v1

### Engineering

- documented the single-object selective-debug workaround for the old Glue scope assertion
- preserved exact ROM signature, timing checks and C fallback for all DIV helpers
- selected targeted 16-bit x86 assembly as the next experiment rather than a full CPU rewrite

## 2026-07-05 — ROW8 and hardware CPU profiling

### Added

- `tools/gbcpu`: real-device LR35902 opcode and memory profiler
- hardware-profile photographs
- guarded division-loop superinstruction
- ROW8 eight-pixel background renderer
- detailed optimization and profiling documents

### Measured

- CPU-only path improved from 21–22 to 28 guest FPS
- packed renderer improved from 6 to 17 guest FPS
- complete path improved from 5 to 13 guest/display FPS
- static GEOS 4-bpp blit remained 44 FPS

### Documented failures

- direct packed rendering without algorithmic reduction
- decoded tile-row cache with 99.98–100% hit rate but lower FPS
- broad timer/housekeeping cleanup as a limited standalone gain

### Repository policy

- current reproducible milestones are included
- ROM images and generated binaries remain excluded
