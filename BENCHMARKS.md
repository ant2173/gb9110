# Hardware benchmarks

All values below were measured on a real Nokia 9110 development target, not inferred from the PC-hosted SDK emulator.

## Why multiple modes exist

A single FPS number is not useful until the frame is decomposed into:

1. Game Boy CPU and hardware timing;
2. PPU line rendering;
3. conversion into GEOS bitmap format;
4. GEOS display transfer;
5. UI/status overhead.

The profiler runs these stages separately.

## GBPROF v0.3

| Mode | Guest FPS | Blit FPS |
|---|---:|---:|
| Core only | 22 | — |
| Renderer + packing, no blit | 6 | — |
| Full 4-bpp path | 5 | 5 |
| Static 4-bpp blit | — | 44 |
| Static 1-bpp blit | — | 29 |

## GBREN v0.4

| Mode | Guest FPS | Blit FPS |
|---|---:|---:|
| Core only | 22 | — |
| PPU only | 7 | — |
| PPU + 4-bpp packing | 6 | — |
| Full 4-bpp path | 5 | 5 |
| Static 4-bpp blit | — | 44 |

## GBFAST v0.5

| Mode | Guest FPS | Blit FPS |
|---|---:|---:|
| Core only | 21 | — |
| Original packed renderer | 6 | — |
| First direct packed renderer | 6 | — |
| Direct packed renderer + blit | 5 | 5 |
| Static 4-bpp blit | — | 44 |

## Conclusions

### 1. Timers were not the limiting factor

The early hardware frontend reported roughly 3 FPS regardless of whether the requested timer rate was 10, 30, or 60 FPS. That was not a timer bug: the work itself took longer than the requested period.

### 2. GEOS blitting is not the dominant cost

A static 4-bpp image can be transferred at roughly 44 FPS. The complete emulated frame is much slower.

### 3. The PPU path dominates

Disabling Peanut-GB line rendering raises throughput from about 7 FPS to about 22 FPS.

### 4. Packing matters, but not enough

The separate conversion from 160 generated pixels to 80 packed 4-bpp bytes costs about one FPS in the current profile.

### 5. The first direct renderer was a useful failure

Writing directly into the packed framebuffer did not help because its inner loops still did too much work. The next renderer changes the number of iterations and sprite selection strategy instead of only changing the destination format.

## Benchmark reporting format

Please report:

```text
Device:
Build:
Mode:
Guest FPS:
Blit FPS:
Visual correctness:
Notes:
```

Run every mode for at least five seconds before recording values.
