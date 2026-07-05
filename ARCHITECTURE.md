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

The throughput loop executes one guest frame, updates counters when needed and queues exactly one next work message. It does not intentionally build a timer backlog.

## 2. Peanut-GB integration

Peanut-GB is adapted for:

- Borland C++ 4.52;
- C89-era syntax;
- 16-bit segmented memory;
- PC/GEOS allocation and application rules.

Stored callback pointers were unreliable in the tested environment. The port therefore uses compile-time direct hook macros for ROM access, LCD output, errors and selected profiling paths.

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

Measured C optimizations include:

- direct fixed-bank instruction fetch;
- direct WRAM stack operations for common `PUSH`/`POP`;
- packed `F` register masks instead of per-flag bit fields;
- hot CB operations handled before the generic decoder;
- simplified interrupt and timer housekeeping;
- selected removal of unnecessary 32-bit arithmetic.

### Guarded DIV v1

An exact ROM signature identifies a hot division-loop iteration. The native path preserves guest registers, stack effects and cycle accounting and falls back when the timing window is unsafe.

### Layered DIV v2

DIV v2 retains the complete helper and adds two smaller guarded blocks:

```text
PREFIX  PUSH AF + RL L/H/C/B     48 guest cycles
FINAL   fixed division epilogue  52 guest cycles
```

The shorter windows can run when a full 144–160-cycle helper would cross an LCD or timer boundary. On hardware this reduced fallback entries by about 62–63% relative to DIV v1 while retaining `CORE ERR = 0` and a valid ROM signature.

These helpers are exact-workload experiments. They are not a substitute for general CPU optimization.

## 6. Renderer evolution

### Original path

The upstream line renderer produced pixels, then the frontend packed them for GEOS. Full performance was about 5 FPS.

### GBTABLE

A lookup-table renderer emits four packed pixels at once and prepares sprite lists per scanline. This reached 12 FPS without blit and 9 FPS with blit.

### Failed tile cache

A separate decoded tile-row cache achieved almost perfect hit rates but was slower. The 16-bit compiler and segmented-memory access made the lookup path more expensive than direct compact decoding.

### ROW8

ROW8 processes 20 groups of eight pixels per scanline rather than 40 groups of four. `SCX & 7` and tile-addressing mode are selected once per line. Previous tile-row data is reused, and four output bytes are produced per iteration.

ROW8 reaches 17 FPS without blit and 12 FPS with blit. Combined with DIV v1, the full path reaches 13 FPS.

## 7. Frame scheduling

`GBFS v1.4c` separates guest execution from display work:

```text
FULL  emulate + render + blit every frame
FS2   emulate every frame; render + blit every second frame
FS3   emulate every frame; render + blit every third frame
```

CPU, timers, interrupts, input and game logic continue on every guest frame. Rendering and blitting are skipped together; the frontend does not redraw stale buffers on omitted frames.

Measured rates:

```text
FULL  13 guest / 13 display FPS
FS2   18 guest /  9 display FPS
FS3   21 guest /  7 display FPS
```

## 8. Current profiling architecture

`GBPROF v1.5j` provides three useful views:

- opcode frequency without rendering;
- data-memory traffic without opcode/immediate fetch bytes;
- combined opcode + memory profiling while running FS2.

The large engine is included in the GOC translation unit. Borland debug scope and line records are disabled locally around the engine include, preserving GOC class symbols while avoiding the Nokia SDK Glue scope assertion.

Profiler counters intentionally distort FPS. Their purpose is relative distribution, not clean throughput.

## 9. Current bottleneck model

There is no single dominant subsystem:

- optimized CPU execution is capped near 28 guest FPS;
- ROW8 rendering lowers the no-blit path to 17 FPS;
- the best render-every-frame path is 13 FPS;
- a static blit is 44 FPS;
- FS2 raises guest progress to 18 FPS while displaying 9 FPS.

## 10. Assembly boundary

The first 8086 experiment should not rewrite the whole LR35902 interpreter. It must be:

- narrow and reversible;
- callable or includable without expensive segmented-memory transitions;
- measured against the C path in the same binary;
- verified for registers, flags, PC, SP, cycles and memory effects;
- retained only after real-device evidence.

The profiler-selected candidates are conditional relative branches, immediate ALU operations, high-memory loads/stores and the surrounding dispatch/memory overhead. See [ASM_PLAN.md](docs/ASM_PLAN.md).
