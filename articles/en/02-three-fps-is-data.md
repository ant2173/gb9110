# Three FPS is not failure: profiling a Game Boy emulator on the Nokia 9110

The first hardware run worked.

That sentence deserves emphasis: the Nokia 9110 launched the application, loaded the ROM, executed Peanut-GB, rendered the image, and accepted controls.

It also ran at roughly three frames per second.

Worse, changing the requested speed from 10 to 30 or 60 FPS changed almost nothing. The game and the diagnostic panel visibly flashed.

This was the point where guessing would have been easy and useless.

## Fixing the flicker first

The flashing was not mysterious LCD behavior.

The status routine periodically filled the entire window with white and redrew everything, including the game image. On a slow monochrome device, that erase-and-redraw cycle was visible.

The fix was simple:

- draw the static interface once;
- update only the small rectangles containing changing numbers;
- never clear the whole game window during normal execution.

The flicker disappeared from the profiler design.

## Why every timer mode produced the same FPS

A requested timer frequency is only a request.

If one emulated frame takes longer than the timer period, the device cannot magically execute more frames. It can only fall behind.

The early result—about three FPS in every requested mode—suggested that work duration, not timer configuration, was the limiting factor.

The profiler therefore abandoned the continual timer for measurement. It used a self-queued worker:

1. execute exactly one unit of work;
2. update counters when needed;
3. queue exactly one next unit.

That measures sustainable throughput without building an artificial backlog of timer events.

## Separating the frame

The profiler measured five paths.

### CPU/core only

The LCD renderer was disabled, but Game Boy CPU execution and timing continued.

Result:

```text
21–22 guest FPS
```

This is already below the native Game Boy frame rate. Even a perfect renderer cannot produce 60 FPS until the core becomes faster or frames are skipped.

### Original PPU only

Peanut-GB rendered background, window, and sprites, but the frontend did not pack or display the result.

Result:

```text
7 guest FPS
```

The PPU path was the main bottleneck.

### PPU plus GEOS packing

The generated 160-pixel scanline was converted into 80 packed 4-bpp bytes.

Result:

```text
6 guest FPS
```

Packing cost about one additional FPS.

### Full path

CPU, PPU, packing, and `GrDrawImage()` all ran.

Result:

```text
5 guest FPS
```

### Static display transfer

No emulation was performed. GEOS repeatedly drew an existing bitmap.

Results:

```text
4-bpp bitmap: 44 FPS
1-bpp bitmap: 29 FPS
```

The surprising part was that the monochrome bitmap was slower. “Use one bit because the screen is monochrome” was not a valid optimization on this graphics path.

## The optimization that did nothing

The first renderer rewrite removed the temporary 160-pixel scanline and wrote directly into the packed GEOS framebuffer.

It sounded exactly right.

It produced exactly no measurable improvement:

```text
original packed renderer: 6 FPS
direct packed renderer:   6 FPS
```

The copy was not the dominant problem.

The rewritten inner loop still:

- performed too much work per pixel;
- recalculated tile addresses repeatedly;
- contained many branches;
- scanned all 40 sprites for every scanline.

The destination format changed. The algorithm did not change enough.

That negative result was valuable. It prevented weeks of polishing the wrong optimization.

## The next experiment

The current renderer changes the amount of work:

- four background pixels decoded through a lookup table;
- one 16-bit write for four packed pixels;
- 40 background iterations per scanline instead of 160;
- first-ten-sprites-per-line lists prepared once;
- only sprites touching the current line visited.

The target is not yet 60 FPS. That would be dishonest.

The immediate target is to move the complete path from 5 FPS into a range where adaptive frame skipping can make the test ROM meaningfully playable. After that, the 21–22 FPS CPU ceiling becomes the next problem.

On old hardware, three FPS was not a verdict.

It was the first useful measurement.

---

## Credits

This work uses the Peanut-GB core by Mahyar Koshkouei (`@deltabeard`) and the
Flappy Bird Gameboy Homebrew test ROM by Larold's Retro Gameyard
(`@LaroldsJubilantJunkyard`). The Nokia/GEOS work was made possible by preserved
material from Marcus Gröber (`@mgroeber9110`) and the blueway.Softworks /
#FreeGEOS contributors. Full credits are listed in the repository's
`ACKNOWLEDGEMENTS.md`.

