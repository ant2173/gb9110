# Project status

**Stage:** hardware-verified optimization of one known 32 KiB homebrew ROM.

## Confirmed working

- EC and NC builds with the Nokia 9110 SDK and Borland C++ 4.52
- installation and launch on a real Nokia 9110
- external ROM loading into locked GEOS memory blocks
- adapted Peanut-GB LR35902 execution
- LCD scanline rendering into a 160×144 4-bpp framebuffer
- keyboard input
- clean queued work loop without timer backlog
- CPU/PPU/packing/blit profiler
- opcode and memory-access profiler
- visually verified lookup-table and ROW8 renderers
- guarded division-loop superinstruction

## Current best measurements

| Measurement | Result |
|---|---:|
| CPU only, optimized + DIV | 28 guest FPS |
| ROW8 renderer, no GEOS blit | 17 guest FPS |
| ROW8 full path | 12 guest/display FPS |
| DIV + ROW8 full path | **13 guest/display FPS** |
| Static 4-bpp GEOS blit | 44 FPS |

The original measured complete path was 5 FPS. The current complete path is 2.6× faster.

## Confirmed limitations

- not full-speed
- one ROM filename and 32 KiB layout are assumed
- the DIV superinstruction is tied to an exact ROM signature
- no audio
- no persistent cartridge RAM
- no ROM chooser
- no compatibility database
- benchmark builds contain multiple old paths and are larger than a future release build
- some unrelated Peanut-GB bit-field warnings remain under Borland

## Current engineering conclusion

The project no longer has one overwhelmingly dominant bottleneck. After ROW8 and CPU hot-path work, CPU execution, line rendering, and GEOS display transfer are all significant.

The next practical optimization is frame skipping with separate guest and display rates, followed by a lean build and another profiler-guided CPU/assembly experiment.
