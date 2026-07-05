# From 5 to 13 FPS on a real Nokia 9110

The first complete Game Boy frame path on the Nokia 9110 ran at about 5 FPS. The current best build reaches 13 FPS on the same hardware.

The improvement did not come from one heroic rewrite. It came from a chain of measured experiments:

- direct packing failed;
- a four-pixel lookup-table renderer doubled packed throughput;
- an on-device opcode profiler exposed fixed-ROM fetches, WRAM stack traffic, and a hot division loop;
- CPU hot paths moved core throughput from 22 to 27 FPS;
- a guarded native division iteration reached 28 FPS;
- a tile cache with almost perfect hit rates made the emulator slower;
- an eight-pixel ROW8 renderer finally moved the complete path from 10 to 12 FPS;
- ROW8 plus the division fast path reached 13 FPS.

The tile-cache failure may be the most useful result. “Cache hit” sounded good, but on a segmented 16-bit compiler the pointer arithmetic, checks, and reconstruction cost more than the compact decoder it replaced.

The current machine still cannot run the Game Boy at full speed. But the project now has a working hardware profiler, reproducible benchmark builds, and a measured 2.6× full-path improvement. The next step is to execute every guest frame while displaying fewer frames: guest timing and display timing must finally become separate numbers.
