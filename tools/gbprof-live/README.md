# GBPROF v1.5j — live opcode and memory profiler

This profiler measures the current ROW8/DIV workload directly on a real Nokia 9110. It reports instruction rates, the hottest main and CB opcodes, memory reads/writes by region, guest FPS, display FPS and DIV-helper activity.

## Modes

```text
1  DIV+ROW8 FULL
2  DIV+ROW8 FS2
3  DIV+ROW8 FS3
4  OPCODE CORE        display intentionally frozen
5  MEMORY CORE        display intentionally frozen
6  OPCODE+MEMORY FS2
0  clear current sample
```

Profiling counters intentionally add overhead. FPS from modes 4–6 must not be compared directly with clean benchmark builds.

## Build note

The final working layout keeps the GEOS shell and engine in one object. `GBENG.INC` and `GBPDATA.INC` are included by `GBPROF.GOC`; Borland debug scopes are disabled locally around the large engine include. This avoids the Nokia SDK Glue assertion in `borland.c` while preserving the GOC class symbols.

Build in `C:\PCGEOS\User1\Appl\GBProf` with `BUILD_GBPROF.BAT`. Supply `FLAPPY.GB` separately.
