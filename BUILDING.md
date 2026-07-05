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

## Latest benchmark: GBDIV2 v1.6

```bat
xcopy /E /I experiments\gbdiv2 C:\PCGEOS\User1\Appl\GBDIV2
cd /d C:\PCGEOS\User1\Appl\GBDIV2
BUILD_GBDIV2.BAT
```

Expected output:

```text
GBDIV2.GEO
```

`GBDIV2` is an NC hardware benchmark. Copy the normal `.GEO` to the real communicator.

## Fixed frame-skip benchmark

```bat
xcopy /E /I experiments\gbfs C:\PCGEOS\User1\Appl\GBFS
cd /d C:\PCGEOS\User1\Appl\GBFS
BUILD_GBFS.BAT
```

Expected output:

```text
GBFS.GEO
```

The application is deliberately named `GB FS Test v1.4c` with a unique token to avoid PC/GEOS confusing it with older `GBROW` or `GBSKIP` binaries.

## Live opcode/memory profiler

```bat
xcopy /E /I tools\gbprof-live C:\PCGEOS\User1\Appl\GBProf
cd /d C:\PCGEOS\User1\Appl\GBProf
BUILD_GBPROF.BAT
```

Expected output:

```text
GBPROF.GEO
```

The final working build is a single-object layout. Borland debug scopes are disabled locally around the large included engine so Glue can retain the GOC class symbols without hitting its scope assertion.

## ROW8 reference benchmark

```bat
xcopy /E /I experiments\gbrow C:\PCGEOS\User1\Appl\GBROW
cd /d C:\PCGEOS\User1\Appl\GBROW
BUILD_GBROW.BAT
```

Expected output:

```text
GBROWEC.GEO
GBROW.GEO
```

Use `GBROW.GEO` on hardware.

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

Unrelated Peanut-GB structures still contain bit fields.

Glue may warn that the main text resource is very large. Benchmark binaries intentionally contain multiple A/B paths. A release-oriented build should remove historical modes and split code into movable resources.

## Known historical build traps

- PC/GEOS derives project names from directories; keep `.GOC`, `.GP`, geode name and folder naming consistent.
- Old Borland tools are sensitive to 8.3 filenames.
- Disabling `-v/-y` for the entire GOC object removes class symbols needed by Glue.
- Large local-symbol streams can trigger a Glue assertion in `borland.c`.
- For `GBPROF v1.5j`, debug records are disabled only around the included engine, not globally.

## Never commit

- ROM images;
- Nokia SDK or PC/GEOS proprietary files;
- generated `.GEO`, `.OBJ`, `.EOBJ`, `.EC`, `.SYM`, map, dependency or temporary files;
- local toolchain paths or licensed compiler components.
