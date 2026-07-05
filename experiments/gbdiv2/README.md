# GBDIV2 v1.6 — layered division-helper benchmark

This build compares three CPU paths:

- helpers off;
- DIV v1: one complete guarded iteration helper;
- DIV v2: the original complete helper plus shorter `PREFIX` and `FINAL` helpers.

`PREFIX` collapses `PUSH AF + RL L/H/C/B` into a 48-cycle native block. `FINAL` collapses the fixed 52-cycle epilogue. Every helper validates the exact ROM signature and timing boundary before running; otherwise execution falls back to the normal interpreter.

## Modes

```text
1/2/3  CORE: OFF / DIV v1 / DIV v2
4/5/6  FS2:  OFF / DIV v1 / DIV v2
7/8/9  FULL: OFF / DIV v1 / DIV v2
0      clear measurement window
```

Key real-device result:

| Path | OFF | DIV v1 | DIV v2 |
|---|---:|---:|---:|
| CPU-only guest FPS | 26 | 27 | **28** |
| FS2 guest/display FPS | 17/6 | 16/9 | **18/9** |

In the CPU-only sample, fallback entries fell from 929/s with DIV v1 to 350/s with DIV v2. In FS2 they fell from 619/s to 229/s. See [`docs/GBDIV2_V16_RESULT.md`](../../docs/GBDIV2_V16_RESULT.md).

Build in `C:\PCGEOS\User1\Appl\GBDIV2` with `BUILD_GBDIV2.BAT`. Supply `FLAPPY.GB` separately.
