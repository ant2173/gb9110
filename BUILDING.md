# Building GB9110

## Required environment

The tested build environment is:

- Nokia 9110 SDK / `N9110V10`;
- PC/GEOS build tools;
- Borland C++ 4.52;
- a configured GEOS command prompt.

Known working roots:

```text
C:\PCGEOS\N9110V10
C:\PCGEOS\User1
```

## Current experiment: GBROW v1.3

Copy the current experiment:

```bat
xcopy /E /I experiments\gbrow C:\PCGEOS\User1\Appl\GBROW
cd /d C:\PCGEOS\User1\Appl\GBROW
```

Build both variants:

```bat
BUILD_GBROW.BAT
```

Expected output:

```text
GBROWEC.GEO
GBROW.GEO
```

Use:

- `GBROWEC.GEO` for EC diagnostics in the SDK environment;
- `GBROW.GEO` on the real Nokia 9110.

## CPU profiler

```bat
xcopy /E /I tools\gbcpu C:\PCGEOS\User1\Appl\GBCPU
cd /d C:\PCGEOS\User1\Appl\GBCPU
BUILD_GBCPU.BAT
```

The profiler records 256 main opcodes, 256 CB opcodes, memory-region reads/writes, interrupts, and elapsed GEOS ticks.

## Earlier hardware frontend

```bat
xcopy /E /I src\gbhw C:\PCGEOS\User1\Appl\GBHW
cd /d C:\PCGEOS\User1\Appl\GBHW
BUILD.BAT
```

## ROM placement

No ROM is included in the repository.

The current sources expect a user-supplied 32 KiB ROM named:

```text
FLAPPY.GB
```

SDK target:

```text
C:\PCGEOS\Target\n9110v10.ec\server\World\ExtrApps
```

Real communicator:

```text
World\ExtrApps
```

Place the normal `.GEO` and ROM together on hardware.

## Expected warnings

Borland may report:

```text
Bit fields must be signed or unsigned int
```

The original CPU flag-register bit fields were removed in later builds, but unrelated Peanut-GB structures still contain bit fields.

The linker may also warn that the main text resource is large. Benchmark binaries intentionally contain multiple old and new renderers for A/B testing. A release-oriented build should remove historical modes and split code into movable resources.

## Never commit

- ROM images;
- Nokia SDK or PC/GEOS proprietary files;
- generated `.GEO`, `.OBJ`, `.EOBJ`, `.EC`, map, dependency, or temporary files;
- local toolchain paths or licensed compiler components.
