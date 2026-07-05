# Changelog

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
