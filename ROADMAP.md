# Roadmap

## Completed: platform proof

- [x] build a Nokia 9110 GEOS application
- [x] load an external Game Boy ROM
- [x] adapt Peanut-GB to Borland C++ 4.52
- [x] execute and render a real ROM continuously
- [x] map keyboard input
- [x] run on real Nokia hardware
- [x] eliminate full-window flicker
- [x] measure CPU, PPU, packing and blit independently

## Completed: profiler-driven C optimization

- [x] lookup-table four-pixel renderer
- [x] opcode and memory-access profiler on the device
- [x] direct ROM fetch and WRAM stack paths
- [x] packed CPU flag register
- [x] timing and interrupt cleanup
- [x] guarded DIV v1 superinstruction
- [x] document a failed high-hit-rate tile cache
- [x] eight-pixel ROW8 renderer
- [x] raise the complete path from 5 to 13 FPS

## Completed: decouple guest and display rates

- [x] fixed FS2 and FS3
- [x] independent guest FPS and display FPS
- [x] skip rendering and blitting together on omitted frames
- [x] verify continued guest execution and input
- [x] identify FS2 as the practical fixed mode

## Completed: current-workload profiling and DIV v2

- [x] rate-based live opcode profiler
- [x] data-memory profiler with opcode fetch excluded
- [x] combined profiler under FS2
- [x] shorter guarded DIV `PREFIX` helper
- [x] shorter guarded DIV `FINAL` helper
- [x] reduce DIV fallback rate by about 62–63%
- [x] validate helper equivalence on host and real hardware

## Next: targeted 16-bit x86 assembly

- [ ] inspect Borland output for dispatch, flag math and high-memory access
- [ ] create one same-binary C vs ASM benchmark
- [ ] keep DIV v2 and FS2 as the baseline
- [ ] start with a narrow hot path, not a full CPU rewrite
- [ ] verify register, flag, PC, SP, cycle and memory equivalence
- [ ] require `CORE ERR = 0` and identical game state signatures
- [ ] keep the ASM path only if real hardware gains at least about 1 FPS or materially reduces CPU cost
- [ ] document negative results if the call boundary erases the gain

See [ASM_PLAN.md](docs/ASM_PLAN.md).

## After ASM1

- [ ] decide between a larger dispatch runner and a renderer inner loop based on measured gain
- [ ] evaluate adaptive frame skip
- [ ] create a lean playable build without historical benchmark modes
- [ ] split oversized code resources
- [ ] improve pause/resume lifecycle

## Emulator features

- [ ] ROM chooser
- [ ] variable ROM sizes
- [ ] general MBC validation
- [ ] cartridge RAM and save files
- [ ] persistent configuration
- [ ] compatibility list

## Deferred

- audio
- link cable
- Game Boy Color
- polished installer
- public binary release

Correct, responsive video and timing remain the priority.
