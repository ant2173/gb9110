# GBFS v1.4c hardware result

Device: real Nokia 9110 Communicator  
Build: NC, `GBFS.GEO`  
Date: 2026-07-05

## Result

| Mode | Guest FPS | Blit FPS |
|---|---:|---:|
| FULL | 13 | 13 |
| FS2 | 18 | 9 |
| FS3 | 21 | 7 |

## Interpretation

The ratios are internally consistent:

- FS2 displays one out of two guest frames: `18 / 2 = 9`;
- FS3 displays one out of three guest frames: `21 / 3 = 7`.

Guest CPU execution, timers, input and game logic continue every frame. Rendering and GEOS blitting are skipped together. FS2 is the practical mode; FS3 is visibly coarse.
