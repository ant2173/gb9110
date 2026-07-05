# GBROW v1.3

Current best benchmark build.

## Modes

```text
1  BASE CORE
2  ORIGINAL PACK
3  TABLE PACK
4  TABLE FULL
5  BLIT 4BPP
6  DIV CORE
7  DIV FULL
8  ROW8 PACK
9  ROW8 FULL
0  DIV + ROW8 FULL
```

## Real-hardware result

```text
3 TABLE PACK       13 FPS
8 ROW8 PACK        17 FPS
4 TABLE FULL       10 FPS
9 ROW8 FULL        12 FPS
7 DIV FULL         10 FPS
0 DIV + ROW8 FULL  13 FPS
```

`DIV HITS` confirms the guarded ROM-specific superinstruction is active in modes 6, 7, and 0.

## Build

```bat
BUILD_GBROW.BAT
```

The ROM is not included. Supply a legal 32 KiB test ROM named `FLAPPY.GB`.
