# Draft GitHub launch post

I have started publishing **GB9110**, an experimental Game Boy emulator frontend for the Nokia 9110 Communicator.

A real DMG ROM now:

- loads from the communicator filesystem;
- executes through an adapted Peanut-GB core;
- renders into a GEOS bitmap;
- accepts keyboard input;
- runs on a real Nokia 9110.

It is not full-speed yet, and the repository does not pretend otherwise.

The interesting part is the hardware profile:

```text
CPU/core only:          21–22 FPS
Original PPU only:           7 FPS
PPU + packed conversion:     6 FPS
Full frame with GEOS blit:   5 FPS
Static GEOS 4-bpp blit:     44 FPS
```

The project is now focused on reducing PPU work and then optimizing the LR35902 interpreter for 16-bit x86 and Borland C++ 4.52.

The repository is structured as an open engineering notebook: runnable baseline, hardware profiler, current renderer experiment, build notes, measurements, and failed approaches.

No ROMs or proprietary SDK files are included.

---

## Credits

This work uses the Peanut-GB core by Mahyar Koshkouei (`@deltabeard`) and the
Flappy Bird Gameboy Homebrew test ROM by Larold's Retro Gameyard
(`@LaroldsJubilantJunkyard`). The Nokia/GEOS work was made possible by preserved
material from Marcus Gröber (`@mgroeber9110`) and the blueway.Softworks /
#FreeGEOS contributors. Full credits are listed in the repository's
`ACKNOWLEDGEMENTS.md`.

