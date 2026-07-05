# GB9110 is now on GitHub

I am publishing GB9110 as an open engineering notebook: a Game Boy emulator experiment for the Nokia 9110 Communicator, built with the original PC/GEOS SDK and Borland C++ 4.52.

A real homebrew ROM already loads, executes, renders, and accepts input on actual hardware. The first measured complete frame path ran at about 5 FPS. After device-side profiling and several renderer and CPU experiments, the current best build reaches 13 FPS.

The repository includes:

- the early hardware frontend;
- CPU/PPU/packing/blit profiling tools;
- an on-device opcode and memory profiler;
- the first successful lookup-table renderer;
- the current ROW8 + DIV benchmark build;
- build notes for the historical toolchain;
- measured failures, including a tile cache with nearly 100% hits that still made the emulator slower.

The project is AI-assisted, but every performance claim is verified on the real Nokia. No optimization is called successful because it looks elegant in source code.

This is not a finished emulator release. There is no sound, save RAM, ROM chooser, or general compatibility claim. The next practical step is frame skipping with separate guest and display rates.

The interesting part is not only that a Game Boy ROM runs on a Nokia 9110. It is that the device is old and constrained enough to make every architectural mistake visible.
