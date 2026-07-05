# Acknowledgements

GB9110 exists because several people and communities made their work public, documented old systems, and preserved source code that could easily have disappeared.

## Peanut-GB

Special thanks to **Mahyar Koshkouei (`@deltabeard`)**, author and maintainer of [Peanut-GB](https://github.com/deltabeard/Peanut-GB).

Peanut-GB provided the compact, portable Game Boy core that made this experiment realistic. GB9110 adapts that core to Borland C++ 4.52, C89-era constraints, segmented 16-bit memory, and the PC/GEOS application model.

The modified Peanut-GB files retain their original MIT license notices. Peanut-GB itself also contains work derived from SameBoy; the relevant upstream notices remain part of the source.

## Flappy Bird Game Boy homebrew

Special thanks to **Larold's Retro Gameyard / `@LaroldsJubilantJunkyard`** for creating and publishing [Flappy Bird Gameboy Homebrew](https://github.com/LaroldsJubilantJunkyard/flappy-bird-gameboy), including its source code, tutorial material, and downloadable ROM release.

The `FlappyBird.gb` v1.0.0 homebrew release has been the main development and profiling workload for GB9110. It is especially useful because it is small, open-source, immediately recognizable on screen, uses scrolling and sprites, and exposes both CPU and PPU performance.

The ROM is **not redistributed** with GB9110. Users must obtain it from its original author or supply another ROM they are legally entitled to use.

The original Flappy Bird game concept was created by **Dong Nguyen**. GB9110 is not affiliated with or endorsed by Dong Nguyen, Nintendo, Nokia, Larold's Retro Gameyard, or the Peanut-GB project.

## Nokia 9000/9110 and PC/GEOS preservation

Special thanks to **Marcus Gröber (`@mgroeber9110`)** for preserving and publishing Nokia 9000/9110 software, source code, examples, and technical notes. His [Nokia 9000/9110 application collection](https://github.com/mgroeber9110/nokia9000-apps) and long-running [Communicator software pages](https://mgroeber.de/nokia.htm) are rare and valuable references for anyone working with these devices today.

Thanks also to **blueway.Softworks and all #FreeGEOS contributors** for maintaining the official [PC/GEOS source repository](https://github.com/bluewaysw/pcgeos), build tools, historical source tree, and the browsable [FreeGEOS technical documentation](https://bluewaysw.github.io/pcgeos/).

Without this preservation work, details such as GEOS memory handling, application resources, file APIs, timers, object messaging, EC/NC builds, and Nokia-target conventions would be far harder to reconstruct.

## The broader communities

Thanks to the Game Boy homebrew, emulator-development, retrocomputing, PC/GEOS, and Nokia Communicator communities for keeping old platforms technically alive rather than merely displaying them behind glass.

Any mistakes in the GB9110 port, documentation, benchmarks, or modified code are the responsibility of the GB9110 project, not the people thanked above.


## AI-assisted development

AI tools have assisted with source analysis, patch generation, documentation, and benchmark interpretation. All device measurements, visual checks, and build outcomes reported by GB9110 were performed and verified by the project author on the relevant SDK or real hardware.
