# Architecture notes

## Frontend

The GEOS frontend owns:

- application lifecycle;
- ROM file loading;
- memory block allocation and locking;
- keyboard events;
- work scheduling;
- framebuffer conversion;
- drawing and status UI.

## Core integration

Peanut-GB originally expects callback functions stored in the emulator state.

In the tested Borland/GEOS environment, raw stored callback pointers caused a runtime failure. The port therefore uses compile-time direct hook macros for:

- ROM reads;
- cartridge RAM reads/writes;
- errors;
- LCD line output.

This is less elegant than the upstream interface, but it is currently stable and measurable.

## ROM memory

The 32 KiB test ROM is loaded into two separately allocated and locked 16 KiB GEOS memory blocks.

This avoids embedding the whole ROM in fixed data and avoids overflowing the 64 KiB near-data model.

## Framebuffer

The initial framebuffer lived in uninitialized global data. The hardware frontend moves it into a separate allocated and locked block.

The current display format is:

```text
160 × 144 pixels
4 bits per pixel
80 bytes per line
11,520 bytes per frame
```

A 1-bpp framebuffer was tested but produced a lower static blit rate than 4-bpp on the current GEOS path.

## Scheduling

A continual timer works well in the fast SDK emulator but can accumulate work or obscure the real throughput limit on hardware.

Profiler builds use:

1. execute one unit of work;
2. update measurements when required;
3. queue exactly one next work message.

This measures the maximum sustainable rate without intentionally building a timer backlog.

## Current performance model

Approximate real-hardware results:

```text
CPU/core only                  21–22 FPS
CPU + original PPU                 7 FPS
CPU + PPU + packed conversion      6 FPS
Complete frame with GEOS output    5 FPS
```

The next optimization target is the PPU, followed by LR35902 dispatch and memory access.
