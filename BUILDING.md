# Building GB9110

## Required toolchain

The current source tree is built with:

- Nokia 9110 SDK / `N9110V10`
- PC/GEOS build tools
- Borland C++ 4.52
- a configured command prompt containing the GEOS paths and environment

Known working roots:

```text
C:\PCGEOS\N9110V10
C:\PCGEOS\User1
```

## Build the hardware frontend

Copy the project:

```bat
xcopy /E /I src\gbhw C:\PCGEOS\User1\Appl\GBHW
cd /d C:\PCGEOS\User1\Appl\GBHW
```

Generate makefiles:

```bat
mkmf
```

Build the error-checking version:

```bat
pmake GBHWEC.GEO
```

Build the normal version:

```bat
pmake GBHW.GEO
```

Use:

- `GBHWEC.GEO` for SDK/emulator diagnostics;
- `GBHW.GEO` on the real communicator.

## Build the profiler

```bat
xcopy /E /I tools\gbprof C:\PCGEOS\User1\Appl\GBProf
cd /d C:\PCGEOS\User1\Appl\GBProf
BUILD.BAT
```

## Build the current renderer experiment

```bat
xcopy /E /I experiments\gbtable C:\PCGEOS\User1\Appl\GBTable
cd /d C:\PCGEOS\User1\Appl\GBTable
BUILD_GBTABLE.BAT
```

## ROM placement

No ROM is included.

The current frontend expects a user-provided 32 KiB ROM named:

```text
FLAPPY.GB
```

For the SDK target, place it in:

```text
C:\PCGEOS\Target\n9110v10.ec\server\World\ExtrApps
```

or the corresponding normal target tree.

For the real communicator, place the ROM and `.GEO` file in:

```text
World\ExtrApps
```

## Common warnings

The old compiler emits warnings about bit-field types in the adapted Peanut-GB header. These warnings have been observed in successful builds, but they should not be ignored forever: the ABI assumptions need a permanent documented solution.

A warning that a text resource is large is also expected. The code currently remains monolithic; splitting the adapted core into movable code resources is a later task.

## Do not publish

Do not commit:

- ROM files;
- SDK binaries;
- Nokia or PC/GEOS proprietary files;
- generated `.GEO`, `.OBJ`, `.EOBJ`, `.EC`, dependency, or map files.
