# Project status

**Stage:** hardware-verified optimization of one known 32 KiB homebrew ROM; transition from C hot paths to targeted 8086 assembly.

## Confirmed working

- NC and selected EC builds with the Nokia 9110 SDK and Borland C++ 4.52
- installation and launch on a real Nokia 9110
- external ROM loading into locked GEOS memory blocks
- adapted Peanut-GB LR35902 execution
- LCD scanline rendering into a 160×144 4-bpp framebuffer
- keyboard input
- clean queued work loop without timer backlog
- CPU/PPU/packing/blit profiler
- original and current live opcode/memory profilers
- lookup-table and ROW8 renderers
- guarded DIV v1 and layered DIV v2 helpers
- fixed FULL / FS2 / FS3 display scheduling
- separate guest and blit FPS counters

## Current best measurements

| Measurement | Result |
|---|---:|
| CPU only, DIV v2 | **28 guest FPS** |
| ROW8 renderer, no GEOS blit | 17 guest FPS |
| Best render-every-frame path | **13 guest/display FPS** |
| Fixed FS2 + DIV v2 | **18 guest / 9 display FPS** |
| Fixed FS3 | 21 guest / 7 display FPS |
| Static 4-bpp GEOS blit | 44 FPS |

The original measured complete path was 5 FPS. The best full-display path is 2.6× faster; FS2 advances guest time 3.6× faster than the original path while displaying 9 FPS.

## Current workload evidence

- live opcode rate: 28,126 opcodes/s in opcode-core mode
- leading opcode: `JR NZ,r8` at 13% in the captured window
- data memory: 19,135 reads/s and 7,759 writes/s
- WRAM dominates data traffic
- DIV v2 reduced fallback entries from 929/s to 350/s in CPU-only and 619/s to 229/s in FS2
- all recorded DIV v2 samples: `CORE ERR = 0`, `DIV SIG = OK`

## Confirmed limitations

- not full-speed
- one ROM filename and 32 KiB layout are assumed
- DIV helpers are tied to an exact ROM signature
- fixed frame skip is not yet adaptive
- no audio
- no persistent cartridge RAM
- no ROM chooser
- no compatibility database
- benchmark builds contain multiple A/B paths and oversized code resources
- some unrelated Peanut-GB bit-field warnings remain under Borland

## Current engineering conclusion

There is no longer one overwhelmingly dominant C subsystem. CPU execution, ROW8 rendering and GEOS transfer all matter. Fixed FS2 provides the best practical balance, and DIV v2 has substantially reduced the remaining known division-loop fallback.

The next controlled experiment is a **small 16-bit x86 assembly fast path** with C fallback and same-binary A/B modes. A full LR35902 rewrite or a full renderer rewrite is not justified yet.
