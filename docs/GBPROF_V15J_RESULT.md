# GBPROF v1.5j hardware result

Device: real Nokia 9110 Communicator  
Build: NC, `GBPROF.GEO`  
Date: 2026-07-05

## Opcode-core mode

```text
Guest FPS: 26
OP/s:      28,126
DIV/s:        870
```

| Opcode | Per second | Share |
|---:|---:|---:|
| `20` | 3,775 | 13% |
| `F0` | 2,228 | 7% |
| `80` | 1,720 | 6% |
| `E6` | 1,247 | 4% |
| `C6` | 1,195 | 4% |
| `28` | 990 | 3% |
| `E0` | 954 | 3% |
| `21` | 840 | 3% |

Top CB sample:

```text
CB 14  approximately 953/s
CB 15  approximately 953/s
CB 10  approximately 816/s
```

## Memory-core mode

Opcode/immediate fetch is excluded.

```text
Guest FPS: 24
Reads/s:   19,135
Writes/s:   7,759
DIV/s:        848
```

| Area | Reads/s | Writes/s |
|---|---:|---:|
| WRAM | 12,408 | 6,862 |
| HRAM | 4,657 | 69 |
| I/O | 2,070 | 828 |

## Combined profile + FS2

```text
Guest FPS: 15
Blit FPS:   7
OP/s:      22,501
Reads/s:   11,114
Writes/s:   6,145
DIV/s:        224
CORE ERR:     0
```

Profiling overhead is intentionally high. The distribution, not profiler FPS, is the result.
