# GBDIV2 v1.6 hardware result

Device: real Nokia 9110 Communicator  
Build: NC, `GBDIV2.GEO`  
Date: 2026-07-05

## CPU-only comparison

| Mode | Guest FPS | FULL/s | PREFIX/s | FINAL/s | FALLBACK/s |
|---|---:|---:|---:|---:|---:|
| 1 CORE OFF | 26 | 0 | 0 | 0 | 0 |
| 2 CORE V1 | 27 | 403 | 0 | 0 | 929 |
| 3 CORE V2 | **28** | 413 | 635 | 54 | **350** |

DIV v2 reduced fallback entries by about 62% relative to DIV v1.

## FS2 comparison

| Mode | Guest FPS | Blit FPS | FULL/s | PREFIX/s | FINAL/s | FALLBACK/s |
|---|---:|---:|---:|---:|---:|---:|
| 4 FS2 OFF | 17 | 6 | 0 | 0 | 0 | 0 |
| 5 FS2 V1 | 16 | 9 | 268 | 0 | 0 | 619 |
| 6 FS2 V2 | **18** | **9** | 272 | 415 | 35 | **229** |

DIV v2 reduced fallback entries by about 63% relative to DIV v1 and retained the best practical 18 guest / 9 display FPS result.

The isolated FS2 OFF and V1 guest-FPS samples are not monotonic, so a one-FPS difference should not be treated as a universal speed law. The helper-rate change and the 18/9 V2 result are the stronger findings.

## Correctness indicators

Every captured mode reported:

```text
CORE ERR: 0
DIV SIG:  OK
```

Host validation also compared 200,000 randomized helper states and 300 emulated frames across helpers-off, DIV v1 and DIV v2 instances.
