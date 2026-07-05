# Contributing

GB9110 is in the measurement and architecture phase. Small changes with clean A/B evidence are more valuable than broad rewrites.

## Useful contributions

- Borland C++ 4.52 assembly/code-generation analysis;
- a benchmark isolating one cost;
- a renderer change with pixel-for-pixel comparison;
- 16-bit C portability fixes;
- GEOS memory, resource, or lifecycle corrections;
- LR35902 dispatch and timing optimization;
- reproducible Nokia 9110 hardware results;
- documentation that makes the historical toolchain easier to use.

## Pull-request rules

1. Change one subsystem or hypothesis at a time.
2. Include baseline and new measurements.
3. Prefer A/B modes in the same binary.
4. State whether image and controls remained correct.
5. Do not include ROMs, SDK files, or generated binaries.
6. Preserve all upstream license notices.
7. Do not call a change faster until it is measured.
8. Keep failed experiments documented when they teach something.

## Performance report

```text
Device:
Toolchain:
Build:
Baseline mode and FPS:
New mode and FPS:
Display/blit FPS:
Visual differences:
Input differences:
Memory/code-size changes:
Compiler warnings:
Run duration:
```
