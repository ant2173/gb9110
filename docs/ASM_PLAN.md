# ASM1 plan: first targeted 16-bit x86 experiment

## Goal

Measure whether a narrow 8086/186-compatible fast path can improve the current real-device baseline without turning the project into an untestable full CPU rewrite.

Baseline:

```text
CPU-only DIV v2:       28 guest FPS
FS2 + DIV v2:          18 guest / 9 display FPS
Render-every-frame:    13 guest/display FPS
```

## Why assembly now

The main C-level wins have been tested:

- direct memory hot paths;
- packed flags;
- renderer loop reduction;
- fixed frame skip;
- long and short guarded DIV helpers;
- current-workload profiling.

DIV v2 cut fallback substantially but did not move the CPU ceiling beyond 28 FPS. Remaining work is spread across dispatch, flag handling, branches and data-memory access.

## First candidate set

The current opcode-core sample is led by:

```text
20  JR NZ,r8
F0  LDH A,(a8)
80  ADD A,B
E6  AND d8
C6  ADD A,d8
28  JR Z,r8
E0  LDH (a8),A
21  LD HL,d16
```

The first build should not necessarily create eight separate far-call handlers. On 16-bit segmented x86, call and resource overhead may exceed the saved instructions. Preferred designs, in order:

1. inline assembly inside the existing near CPU runner;
2. one small near assembly block handling a compact hot dispatch subset;
3. a short multi-opcode runner that stays inside one code resource;
4. only then a separate `.asm` module.

## ASM1 benchmark design

One binary, same ROM state, selectable modes:

```text
CORE C + DIV v2
CORE ASM1 + DIV v2
FS2 C + DIV v2
FS2 ASM1 + DIV v2
```

Counters:

```text
Guest FPS
Blit FPS
ASM hits/s
ASM fallbacks/s
CORE ERR
state signature
```

## Correctness requirements

For every ASM-handled instruction or block, compare:

- A, F, B, C, D, E, H, L;
- PC and SP;
- cycle count;
- memory reads and writes;
- interrupt/timer boundary behavior;
- framebuffer/game-state signature after repeated frames.

The C interpreter remains the fallback and reference.

## Keep / reject threshold

Keep ASM1 only if the real Nokia shows one of:

- at least about +1 stable guest FPS in CPU-only or FS2;
- a material reduction in measured CPU time at unchanged FPS;
- a clear code-size or call-boundary advantage that enables the next runner.

Reject and document it if:

- speed is unchanged;
- a far/near call boundary erases the arithmetic win;
- code-resource pressure or Glue complexity outweighs the result;
- correctness diverges.

## Explicitly out of scope for ASM1

- full LR35902 interpreter rewrite;
- full ROW8 rewrite;
- sound;
- general ROM compatibility;
- replacing the safe timing model with approximate timing.
