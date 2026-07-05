# Architecture

## 1. GEOS frontend

The frontend owns:

- application lifecycle and object messages;
- ROM file loading;
- GEOS memory allocation and locking;
- keyboard events;
- one-at-a-time queued work scheduling;
- framebuffer storage and GEOS drawing;
- benchmark UI and counters.

The maximum-throughput loop executes one guest unit, updates counters when needed, and queues exactly one next work message. It does not intentionally build a timer backlog.

## 2. Peanut-GB integration

Peanut-GB is adapted for:

- Borland C++ 4.52;
- C89-era syntax;
- 16-bit segmented memory;
- PC/GEOS allocation and application rules.

Stored callback pointers were unreliable in the tested environment. The port therefore uses compile-time direct hook macros for ROM access, LCD output, errors, and selected profiling paths.

## 3. ROM memory

The current 32 KiB ROM is loaded into two separately allocated and locked 16 KiB blocks:

```text
0000–3FFF  fixed bank
4000–7FFF  switchable bank
```

This avoids placing the ROM in fixed near data and respects the 64 KiB data-model constraints.

The current benchmark is deliberately ROM-specific. General MBC and variable-size ROM handling are future work.

## 4. Emulator state and framebuffer

The display buffer is allocated outside fixed global data:

```text
160 × 144 pixels
4 bits per pixel
80 bytes per line
11,520 bytes per frame
```

A static 4-bpp GEOS transfer reaches about 44 FPS on the device. A tested 1-bpp path reached only 29 FPS, so 4-bpp remains the preferred output format despite the monochrome screen.

## 5. CPU hot paths

Measured optimizations include:

- direct fixed-bank instruction fetch;
- direct WRAM stack operations for common `PUSH`/`POP`;
- packed `F` register masks instead of per-flag bit fields;
- hot CB operations handled before the generic decoder;
- simplified interrupt and timer housekeeping;
- selected removal of unnecessary 32-bit arithmetic.

A guarded superinstruction recognizes an exact division-loop ROM signature and executes one guest iteration in native C while preserving registers, stack effects, cycles, and safe fallback behavior.

## 6. Renderer evolution

### Original path

The upstream line renderer produced pixels, then the frontend packed them for GEOS. Full performance was about 5 FPS.

### GBTABLE

A lookup-table renderer emits four packed pixels at once and prepares sprite lists per scanline. This reached 12 FPS without blit and 9 FPS with blit.

### Failed tile cache

A separate decoded tile-row cache achieved almost perfect hit rates but was slower. The 16-bit compiler and segmented-memory access made the lookup path more expensive than direct compact decoding.

### ROW8

The current renderer processes 20 groups of eight pixels per scanline rather than 40 groups of four. `SCX & 7` and tile-addressing mode are selected once per line. Previous tile-row data is reused, and four output bytes are produced per iteration.

ROW8 reaches 17 FPS without blit and 12 FPS with blit. Combined with the CPU superinstruction, the full path reaches 13 FPS.

## 7. Current bottleneck model

At the present stage there is no single dominant subsystem:

- optimized CPU execution is capped near 28 guest FPS;
- ROW8 rendering lowers the no-blit path to 17 FPS;
- the complete path is 13 FPS;
- a static blit is 44 FPS.

The next architectural step is to run every guest frame while rendering and blitting only selected frames.
