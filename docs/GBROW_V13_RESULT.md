# GBROW v1.3 result

## A/B measurements

| Mode | Guest FPS | Blit FPS |
|---|---:|---:|
| TABLE PACK | 13 | — |
| ROW8 PACK | **17** | — |
| TABLE FULL | 10 | 10 |
| ROW8 FULL | **12** | 12 |
| DIV FULL | 10 | 10 |
| DIV + ROW8 FULL | **13** | 13 |

## Interpretation

ROW8 reduced scanline background work from 40 four-pixel groups to 20 eight-pixel groups. Packed frame time fell from about 76.9 ms to 58.8 ms. The complete ROW8 path fell from about 100 ms to 83.3 ms.

Combining ROW8 and the guarded division superinstruction reached about 76.9 ms per complete guest/display frame.

## Correctness

The hardware test confirmed:

- game starts;
- frame is visually correct;
- input works;
- background, bird, pipes, and menu remain intact;
- DIV counter advances in the combined mode.

## Current headline

```text
First complete path:   5 FPS
Current complete path: 13 FPS
Improvement:           2.6×
```
