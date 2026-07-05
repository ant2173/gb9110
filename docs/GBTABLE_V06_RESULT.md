# GBTABLE v0.6 — historical result

`GBTABLE v0.6` was the first decisive hardware optimization.

| Mode | Before | GBTABLE |
|---|---:|---:|
| Packed renderer | 6 FPS | 12 FPS |
| Complete path | 5 FPS | 9 FPS |
| Static 4-bpp blit | 44 FPS | 44 FPS |

The unchanged static blit proved the gain came from the renderer. This milestone established the project's measurement-driven method.

Later ROW8 work reached 17 FPS packed and 12 FPS full; DIV + ROW8 reached 13 FPS complete. See [OPTIMIZATION_HISTORY.md](OPTIMIZATION_HISTORY.md).
