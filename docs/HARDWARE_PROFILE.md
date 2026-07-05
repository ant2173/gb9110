# LR35902 hardware profile

`GBCPU v0.7` ran on a real Nokia 9110 and profiled 180 guest frames.

## Summary

```text
Profiled frames:      180
Main instructions:    317,034
CB instructions:       37,440
Memory reads:         566,046
Memory writes:         71,442
Serviced interrupts:    1,260
Profiler ticks:           687
Profiler FPS:              15
```

Profiler FPS includes heavy counter overhead and must not be compared directly with the clean CPU-only benchmark. The useful data is the relative distribution.

## Main opcode top ten

| Rank | Opcode | Meaning | Count | Approx. share |
|---:|---:|---|---:|---:|
| 1 | `CB` | CB prefix | 37,440 | 11% |
| 2 | `20` | `JR NZ,r8` | 25,920 | 8% |
| 3 | `3D` | `DEC A` | 16,020 | 5% |
| 4 | `F0` | `LDH A,(a8)` | 12,600 | 3% |
| 5 | `B7` | `OR A` | 11,970 | 3% |
| 6 | `C5` | `PUSH BC` | 10,980 | 3% |
| 7 | `F1` | `POP AF` | 10,980 | 3% |
| 8 | `F5` | `PUSH AF` | 10,980 | 3% |
| 9 | `C1` | `POP BC` | 10,350 | 3% |
| 10 | `38` | `JR C,r8` | 10,080 | 3% |

Together, these opcodes account for roughly half of all main instructions.

## CB-prefixed hot set

| Opcode | Meaning | Count |
|---:|---|---:|
| `14` | `RL H` | 9,180 |
| `15` | `RL L` | 9,180 |
| `10` | `RL B` | 8,640 |
| `11` | `RL C` | 8,640 |

These four commands represent about 95% of all CB-prefixed instructions in the captured workload. They belong to a hot 16-bit division routine.

## Memory distribution

### Reads

| Region | Count | Share |
|---|---:|---:|
| ROM0 | 426,402 | 75.3% |
| WRAM | 102,924 | 18.2% |
| HRAM | 24,300 | 4.3% |
| I/O | 12,420 | 2.2% |
| Other regions | 0 | 0% |

### Writes

| Region | Count | Share |
|---|---:|---:|
| WRAM | 66,222 | 92.7% |
| I/O | 4,860 | 6.8% |
| HRAM | 360 | 0.5% |
| Other regions | 0 | 0% |

Opcode and immediate-byte fetches are included in ROM reads.

## Optimizations derived from this profile

- direct fixed-bank instruction fetch;
- fast WRAM stack path;
- direct handling of hot CB rotates;
- packed flag register;
- high-memory read fast paths;
- exact-signature division-loop superinstruction.

The profile converted CPU optimization from a generic list into a measured workload-specific plan.
