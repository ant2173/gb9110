# Contributing

This project is at the profiling and architecture stage. Small, measurable changes are more valuable than broad rewrites.

## Good contributions

- a benchmark that isolates one cost;
- a change with before/after hardware numbers;
- Borland 4.52 code-generation analysis;
- fixes for C89/16-bit portability;
- reduced renderer work with visual comparison;
- GEOS lifecycle or memory corrections;
- documentation that makes the build reproducible.

## Pull-request rules

1. Change one subsystem at a time.
2. Include real-hardware or SDK-emulator results.
3. State whether the image remained correct.
4. Do not include ROMs, SDK files, or generated binaries.
5. Preserve third-party license notices.
6. Do not call a change “faster” without measurements.

## Performance report

```text
Device:
Toolchain:
Build:
Baseline mode/FPS:
New mode/FPS:
Visual differences:
Memory changes:
Warnings:
```
