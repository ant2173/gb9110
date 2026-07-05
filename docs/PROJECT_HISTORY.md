# Project history

## 1. GEOS shell and framebuffer

The project began by proving that a custom application could build, install, draw, and update under the Nokia 9110 SDK.

## 2. Peanut-GB adaptation

The core was modified for Borland C++ 4.52, C89-era constraints, direct hooks, and segmented GEOS memory.

## 3. First real ROM frame

A 32 KiB ROM loaded from the filesystem and completed a real frame:

```text
CPU steps: 6005
LCD lines:  144
Core errors: 0
```

## 4. Continuous execution and input

The SDK frontend ran continuously and accepted keyboard input. The real phone exposed the true limit: about 3 FPS in the early unoptimized application, with visible flicker.

## 5. Hardware profiler

Full-window clearing was removed, work became self-queued, and CPU/PPU/packing/blit stages were measured independently. The initial complete path was about 5 FPS.

## 6. Renderer redesign

Direct packing failed. Four-pixel LUT rendering succeeded, raising the complete path to 9 FPS.

## 7. CPU profiling

A real-device opcode and memory profiler identified hot branches, stack traffic, fixed-bank ROM fetches, and a CB-heavy division routine.

## 8. CPU optimization

Direct ROM and stack paths, packed flags, timing cleanup, and a guarded division superinstruction raised the CPU-only path to 28 FPS.

## 9. Cache failure and ROW8 success

A decoded tile cache slowed the program despite nearly perfect hit rates. A simpler ROW8 design halved the renderer loop count and raised the complete path to 12 FPS.

## 10. Current result

Combining DIV and ROW8 reaches 13 guest/display FPS on the real Nokia 9110: 2.6× the first complete measured path.
