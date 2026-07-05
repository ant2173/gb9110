# Roadmap

## Completed: platform proof

- [x] build a Nokia 9110 GEOS application
- [x] load an external Game Boy ROM
- [x] adapt Peanut-GB to Borland C++ 4.52
- [x] execute a real ROM frame
- [x] render continuously
- [x] map keyboard input
- [x] run on real Nokia hardware
- [x] eliminate full-window flicker
- [x] measure CPU, PPU, packing, and blit independently

## Completed: profiler-driven optimization

- [x] lookup-table four-pixel renderer
- [x] opcode and memory-access profiler on the device
- [x] direct ROM fetch and WRAM stack paths
- [x] packed CPU flag register
- [x] timing and interrupt cleanup
- [x] guarded division-loop superinstruction
- [x] document a failed high-hit-rate tile cache
- [x] eight-pixel ROW8 renderer
- [x] raise the complete path from 5 to 13 FPS

## Next: make the current ROM feel faster

- [ ] fixed frame skip: render every second or third guest frame
- [ ] adaptive frame skip
- [ ] separate guest FPS and display FPS counters
- [ ] verify input latency and game timing under skipped frames
- [ ] skip PPU callbacks entirely on non-rendered frames where safe
- [ ] create a lean build containing only the current paths
- [ ] split oversized code resources

## Next CPU investigation

- [ ] re-profile the current ROW8/DIV build
- [ ] measure the remaining instruction families
- [ ] inspect Borland assembly for dispatch and memory access
- [ ] test a longer safe CPU runner with fewer C call boundaries
- [ ] evaluate a small 16-bit x86 assembly hot loop
- [ ] expand or remove ROM-specific superinstructions based on evidence

## Emulator features

- [ ] ROM chooser
- [ ] variable ROM sizes
- [ ] general MBC validation
- [ ] cartridge RAM and save files
- [ ] pause/resume lifecycle
- [ ] persistent configuration
- [ ] compatibility list

## Deferred

- audio
- link cable
- Game Boy Color
- polished installer
- public binary release

Audio remains deliberately deferred. Correct, responsive video and timing come first.
