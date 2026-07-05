# GBFS v1.4c — frame-skip benchmark

This deliberately renamed PC/GEOS application separates guest execution from display updates and avoids collisions with older `GBROW.GEO` / `GBSKIP.GEO` files on the device.

## Modes

```text
1  FULL: emulate, render and blit every guest frame
2  FS2:  emulate every frame, render/blit one out of two
3  FS3:  emulate every frame, render/blit one out of three
0  FULL alias
```

Measured on a real Nokia 9110:

| Mode | Guest FPS | Display FPS |
|---|---:|---:|
| FULL | 13 | 13 |
| FS2 | 18 | 9 |
| FS3 | 21 | 7 |

Build in `C:\PCGEOS\User1\Appl\GBFS` with `BUILD_GBFS.BAT`. Supply your own 32 KiB test ROM as `FLAPPY.GB`; ROMs are not part of this repository.
