# A Game Boy emulator on the Nokia 9110: the first real frame

The Nokia 9110 Communicator is not an obvious emulation target.

Its development environment belongs to another world: PC/GEOS, Borland C++ 4.52, segmented memory, movable resources, near and far pointers, and a build chain whose error messages sometimes feel archaeological.

That is exactly why I wanted to try it.

The goal was simple to state and difficult to fake: execute a real Game Boy ROM on a real Nokia 9110, render its screen, and control it from the communicator keyboard.

## Building the staircase

I did not begin by dropping an emulator core into the SDK.

The first stages were deliberately boring:

1. create a GEOS application;
2. draw a static image;
3. update it from a timer;
4. allocate a full 160×144 framebuffer;
5. run a synthetic LR35902-like workload;
6. only then integrate Peanut-GB.

That staircase mattered. Every time the program failed, there was a known lower level that still worked.

## Adapting Peanut-GB to 1990s C

Peanut-GB is a compact and fast Game Boy emulator core, but the upstream code expects a much more modern C environment than Borland C++ 4.52.

The first successful adaptation used compile-time direct hooks for:

- ROM reads;
- cartridge RAM access;
- error reporting;
- LCD scanline output.

This was not just an optimization. Raw callback pointers stored in the emulator state caused a runtime failure in the tested environment. A direct-hook build proved stable.

## The 64 KiB trap

The first attempt embedded the entire 32 KiB ROM in the application.

That was a mistake.

In a 16-bit memory model, the ROM competed with the emulator state, framebuffer, stack, and other globals inside a limited near-data world. The application built, but startup became unreliable.

The working design loads the ROM from the GEOS filesystem into two separately allocated and locked 16 KiB blocks.

The framebuffer later moved out of fixed uninitialized data as well.

## The bug that was not in the emulator

One of the most confusing failures was a build that appeared to hang before drawing anything.

The ROM loader was blamed. Then memory allocation. Then the core.

The real cause was application identity.

GEOS uses permanent names and tokens, and reusing an old identity allowed stale application state to interfere with the new diagnostic build. A fresh app name and token immediately produced a visible staged startup screen.

That lesson is now part of the project’s diagnostic discipline: new experiments get fresh identities.

## The first real frame

The successful diagnostic build reported:

- ROM loaded;
- `gb_init()` succeeded;
- the CPU executed thousands of steps;
- the PPU produced 144 LCD lines;
- no core error was reported;
- a real guest frame completed.

The first screen was not yet a game. It was proof that every layer had connected:

```text
GEOS app
  -> file loader
  -> ROM memory
  -> Peanut-GB CPU
  -> Game Boy PPU
  -> scanline conversion
  -> GEOS bitmap
  -> Nokia display
```

## From one frame to a running game

A conservative 10 FPS loop came next, then 30 FPS, then a 60-guest/30-display design. In the SDK emulator it reached about 59 guest FPS.

Keyboard input was added only after the timing loop was stable:

- arrows for the D-pad;
- Space or Z for A;
- X for B;
- Enter for Start;
- Tab for Select.

At that point the ROM was running and controllable.

The next step was the one that mattered most: real hardware.

And real hardware was much less polite than the PC emulator.

That is the subject of the next article.

---

## Credits

This work uses the Peanut-GB core by Mahyar Koshkouei (`@deltabeard`) and the
Flappy Bird Gameboy Homebrew test ROM by Larold's Retro Gameyard
(`@LaroldsJubilantJunkyard`). The Nokia/GEOS work was made possible by preserved
material from Marcus Gröber (`@mgroeber9110`) and the blueway.Softworks /
#FreeGEOS contributors. Full credits are listed in the repository's
`ACKNOWLEDGEMENTS.md`.

