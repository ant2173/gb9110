# Eighteen guest FPS, nine visible: the Nokia 9110 emulator reaches the assembly boundary

The previous milestone rendered every Game Boy frame and reached 13 FPS on a real Nokia 9110. That number was honest, but it also forced CPU execution, line rendering and GEOS transfer to happen for every guest frame.

The next experiment separated guest time from display time.

`GBFS v1.4c` continued to execute CPU, timers, interrupts, input and game logic every frame, while rendering and blitting only selected frames:

```text
FULL  13 guest / 13 display FPS
FS2   18 guest /  9 display FPS
FS3   21 guest /  7 display FPS
```

FS2 became the useful compromise. The game advances faster without turning the screen into the seven-FPS slideshow of FS3.

The current workload was then profiled again, this time as rates rather than one cumulative 180-frame report. The live profiler measured 28,126 opcodes per second in CPU-only mode. `JR NZ,r8` alone represented 13% of the captured stream; `LDH`, immediate ALU operations and another conditional branch followed. A separate profile, excluding opcode fetch bytes, measured 19,135 data reads and 7,759 writes per second, dominated by WRAM and high memory.

The profile also showed that the first ROM-specific division helper still fell back to the ordinary interpreter frequently. A second generation kept the complete helper but added two shorter safe blocks: a 48-cycle prefix and a 52-cycle epilogue.

On hardware, DIV v2 reduced fallback entries from 929 to 350 per second in CPU-only mode, and from 619 to 229 under FS2. The clean CPU path remained at 28 FPS, while the practical FS2 result held 18 guest / 9 display FPS. The improvement was not a miracle FPS jump; it was evidence that the last large, obvious C superinstruction opportunity had been harvested safely.

That changes the next decision. Rewriting the whole LR35902 interpreter in assembly would be reckless. Rewriting nothing because “the compiler should handle it” would be equally lazy. The next build will therefore be a narrow 16-bit x86 A/B experiment, with the C interpreter preserved as reference and fallback.

This is the point where the project stops asking whether assembly might help and starts asking exactly where the call boundary, flag math and segmented memory make it worthwhile.
