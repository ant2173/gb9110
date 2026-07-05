# GBCPU v0.7

Real-device LR35902 opcode and memory-access profiler.

## Captured data

- all 256 main opcode counts;
- all 256 CB-prefixed opcode counts;
- total instructions;
- reads and writes by Game Boy memory region;
- serviced interrupts;
- elapsed GEOS ticks.

## Workflow

1. Start the game.
2. Enter a representative state.
3. Press `P`.
4. Wait for the 180-frame profile.
5. Open report pages:

```text
S  summary
O  main opcodes
C  CB opcodes
M  memory distribution
G  resume game
P  profile again
Q  quit
```

Profiler FPS includes counter overhead. Use the counts and relative shares, not its FPS, to guide optimization.

The ROM is not included.
